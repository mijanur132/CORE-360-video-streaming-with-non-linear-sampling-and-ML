
#include"path.h"
#include"image.h"
#include<stdio.h>
#include <stdlib.h>
#include"config.h"
#include "ERI.h"
#include <chrono>
#include "pixelCalculation.h"
#include <C:\opencv\build\include\opencv2\videoio.hpp>
#include <C:\opencv\build\include\opencv2\opencv.hpp>
#include <C:\opencv\build\include\opencv2\core\core.hpp>
#include <C:\opencv\build\include\opencv2\highgui\highgui.hpp>

using namespace std;
using namespace cv;
using namespace std::chrono;


Path::Path() {
	cout << "One Path created" << endl;
}

Path::~Path() {
	cout << "One Path deleted" << endl;
}



void Path::AppendCamera(PPC newCam, int framesN)
{
	cams.push_back(newCam);
	segmentFramesN.push_back(framesN);
}



void Path::PlayBackPathStillImage(Mat eriPixels, ERI eri, Mat convPixels) 
{
	//to play back every camera for one frame segmentFramesN[segi] needs to be 2
	//it does not reach last frame
	Mat erivis = cv::Mat::zeros(eri.h, eri.w, IMAGE_TYPE);
	cout << cams.size() - 1 << endl;	
	for (int segi = 0; segi < cams.size()-1; segi++)
	{
		for (int fi = 0; fi < segmentFramesN[segi]-1; fi++) 
		{
			PPC ppcL(cams[segi]);
			PPC ppcR(cams[segi +1]);
			PPC interPPC;
			interPPC.SetInterpolated(&ppcL, &ppcR, fi, segmentFramesN[segi]);
			eri.ERI2Conv(eriPixels, convPixels,interPPC);
			eri.VisualizeNeededPixels(erivis, &interPPC);						
			imshow("ERIpixelsNeeded", erivis);
			imshow("outimage", convPixels);
			waitKey(1);
			
		}
	}   	 
	
}

void Path::LoadHMDTrackingData(char* filename, PPC ppc0)
{
	   
	ifstream  file(filename);
	if (!file)
	{
		cout << "error: can't open file: " <<filename<< endl;
		system("pause");
	}
	string   line;
	while (getline(file, line))
	{
		stringstream  linestream(line);
		float timestamp;
		linestream >> timestamp;
		tstamps.push_back(timestamp);
		int frameindex;
		linestream >> frameindex;
		float q[4];
		linestream >> q[0] >> q[1] >> q[2] >> q[3];

		float theta2, v1, v2, v3;
		theta2 = acosf(q[0]);

		v1 = q[1] / sin(theta2);
		v2 = q[2] / sin(theta2);
		v3 = q[3] / sin(theta2);
		
		PPC camera1(ppc0);
		//camera1.PositionAndOrient(V3(0, 0, 0), V3(0, 0, 1), V3(0, 1, 0));
		V3 v(v2, v3, v1);		
		camera1.RotateAboutAxisThroughEye(v, theta2*2*180/PI);
		AppendCamera(camera1,2 );
	}

	cout << "read: " << cams.size() << " cameras from: " << filename << endl;
	
		
}



void Path::PlayBackPathVideo(char* fname, Mat convPixels, int lastFrame)
{	
	VideoCapture cap(fname);
	if (!cap.isOpened()) {
		cout << "Cannot open the video file: " <<fname<< endl;
		waitKey(100000);
		return;

	}	
	ERI eri(cap.get(CAP_PROP_FRAME_WIDTH), cap.get(CAP_PROP_FRAME_HEIGHT));
	Mat erivis = Mat::zeros(eri.h/5,eri.w/5, IMAGE_TYPE);
	int fps = cap.get(CAP_PROP_FPS);
	
	float tstep = 0;
	int segi = 0;
	for (int fi=0; fi<=lastFrame; fi++)
	{  
		Mat frame;
		cap >> frame;		
		if (frame.empty())
		{
			cout << "Can not read video frame: "<<fname<< endl;
			waitKey(100000);
			return;
		}	
		segi = GetCamIndex(fi, fps, segi);
		eri.ERI2Conv(frame, convPixels, cams[segi]);	
		cout << fi << " " << segi << "; ";
		//imshow("outputImage", convPixels);
		eri.VisualizeNeededPixels(erivis, &(cams[segi]));
		cout << "done visualisation" << endl;
		imshow("ERIpixelsNeeded", erivis);
		waitKey(10);
	
	}	   	
	

	//destruct ERI here
}

void Path::PlayBackPathVideoPixelInterval(char* fname, Mat convPixels, int lastFrame)
{

	VideoCapture cap(fname);
	if (!cap.isOpened()) {
		cout << "Cannot open the video file: " << fname << endl;
		waitKey(100000);
		return;

	}
	
	ERI eri(cap.get(CAP_PROP_FRAME_WIDTH), cap.get(CAP_PROP_FRAME_HEIGHT));
	Mat erivis = Mat::zeros(eri.h / 5, eri.w / 5, IMAGE_TYPE);
	int fps = cap.get(CAP_PROP_FPS);
	int totalpixelnumber = eri.h*eri.w;
	PIXELCALCULATION pixelcalc(totalpixelnumber);

	float tstep = 0;
	int segi = 0;
	for (int fi = 0; fi <= lastFrame; fi++)
	{
		Mat frame;
		cap >> frame;
		if (frame.empty())
		{
			cout << "Can not read video frame: " << fname << endl;
			waitKey(100000);
			return;
		}
		segi = GetCamIndex(fi, fps, segi);	
		pixelcalc.GetFramePixelInterval(eri, erivis, &(cams[segi]));		
		cout << "done visualisation" << endl;		
		waitKey(1);

	}

	//pixelcalc.PrintPixelInterval();
	pixelcalc.SaveIntervalTxt();
	cout << "done Interval saving" << endl;
}//most outer loop of playbackpathvideoopixelinterval




int Path::GetCamIndex(int fi, int fps, int segi) {

	int ret = segi;
	cout << fps <<""<<fi<< endl;
	while (tstamps[ret] <= (float)(fi)/(float)(fps))
	{
		ret++;
		if (ret >= tstamps.size())
		{
			cerr<<"Reached the end of the path without finding HMD pos for frame"<<endl;
			waitKey(10000);
			exit(0);

		}

	}

	return ret - 1;

}

void Path::ConvPixel2ERITile(char *fname, int lastFrame, int m, int n, int t)
{
	VideoCapture cap(fname);
	if (!cap.isOpened())
	{
		cout << "Can not open the video file: " << fname << endl;
		waitKey(100000);
		return;
	}
	int scale = 5;
	ERI eri(cap.get(CAP_PROP_FRAME_WIDTH), cap.get(CAP_PROP_FRAME_HEIGHT));
	Mat erivis = Mat::zeros(eri.h / scale, eri.w / scale, IMAGE_TYPE);
	Mat erivis_noscale = Mat::zeros(eri.h, eri.w, IMAGE_TYPE);
	vector <int> Tilebitmap;
	
	for (int i = 0; i < m*n; i++)
	{
		Tilebitmap.push_back(0);
	}
	int fps = cap.get(CAP_PROP_FPS);

	int segi = 0;
	int totaltiles = 0;
	uint64 ERItotal = 0;
	 
	for (int fi = 1; fi < lastFrame+1; fi++)
	{
		Mat frame;
		cap >> frame;
		
		if (frame.empty())
		{
			cout << "Can not read the frame" << endl;
			waitKey(100000);
			return;
		}
		segi = GetCamIndex(fi, fps, segi);
		int pixelI, pixelJ = 0;
		for (int v = 0; v < cams[segi].h; v++) 
		{
			for (int u = 0; u < cams[segi].w; u++) 
			{
				eri.EachPixelConv2ERI(cams[segi], u, v, pixelI, pixelJ);
				int Xtile = floor(pixelJ*m / eri.w); //m*n row and column
				int Ytile = floor(pixelI*n / eri.h);
				int vectorindex = (Ytile)*m + Xtile;
				Tilebitmap.at(vectorindex) = 1;

			}				
		
		}
		
		for (int row = 0; row < erivis.rows; row++)
		{
			for (int col = 0; col < erivis.cols; col++)
			{
				int Xtile = floor(col*m*scale / eri.w); //m*n row and column
				int Ytile = floor(row*n*scale / eri.h);				
				Vec3b insidecolor(25 * (Xtile + 1)*(Ytile + 1), 0, 0);
				erivis.at<Vec3b>(row, col) = insidecolor;

				
			}
		}
		
		if (fi == 1) {
			eri.getERIPixelsCount(erivis_noscale, &(cams[segi]), ERItotal);
		}
		

		if (fi%(fps*t) == 0) 
		{
			//cout <<"frame No: "<< fi << "chunk no: " << fi / (fps*t) << endl;
			for (int i = 0; i < m*n; i++)
			{
				
				if (Tilebitmap[i]==1)
				{
					totaltiles++;
				}
				//cout << "tile: " << Tilebitmap[i] <<", Total-"<<totaltiles<< endl;
				Tilebitmap.at(i)=0;
			}
			waitKey(10);
		}

		
	}//outer for loop for all frame
	cout <<"H: "<< eri.h<<" W: "<< eri.w<<" M: "<<m<<" N: "<<n<<" FPS: "<<fps<<" "<<" T: "<<t<<" Total Tiles: "<<totaltiles<< endl;
	double total_tile_pixel = eri.h*eri.w*fps*t*1UL / (m*n);
	total_tile_pixel = totaltiles * total_tile_pixel*1UL;
	double result = (ERItotal*lastFrame * 100UL /total_tile_pixel) ;
	//cout << "Total_tile_pixel: " << total_tile_pixel << endl;
	//cout << "Total requierd ERI pixel: " << ERItotal*lastFrame << endl;
	cout << "Required and supplied %: " << result << endl;

}

void Path::VDrotationAvg()
{	
	double total_angle=0;
	float angle=0;
	for (int i = 0; i < cams.size(); i++)
	{
		V3 cam1 = cams[i].GetVD();
		V3 cam2 = cams[i + 1].GetVD();
		cout << cam1 <<"; " <<cam2<< endl;
		double angleproduct = cam1 * cam2;
		if (angleproduct <= -1.0) {
			angle=180;
		}
		else if (angleproduct >= 1.0) {
			angle= 0;
		}
		else {
			angle = acos(cam1*cam2)* 180.0f / PI;
		}
		
		total_angle = total_angle + angle;
		cout << "Angle: " << angle << " Total angle: " << total_angle << endl;
	
	}
	
	cout << "No of camera: " << cams.size() << "; Total Angle: " << total_angle << endl;
}

void Path::WriteH264(char* fname, int lastFrame, int codec)
{

	VideoCapture cap(fname);
	if (!cap.isOpened()) 
	{
		cout << "Cannot open the video file: " << fname << endl;
		waitKey(100000);
		return;

	}	

	int frame_width = cap.get(CAP_PROP_FRAME_WIDTH);
	int frame_height = cap.get(CAP_PROP_FRAME_HEIGHT);
	int fps = cap.get(CAP_PROP_FPS);

	std::ostringstream oss;
	oss << fname<<codec <<".avi";
	string filename = oss.str();

	//string filename = "./Video/rollerh264.avi";
	VideoWriter writer;

	//int codec = VideoWriter::fourcc('X','V','I','D');
	writer.set(VIDEOWRITER_PROP_QUALITY,20);
	writer.open(filename, codec, fps,Size(frame_width,frame_height), true);
	
	//cout<<writer.get(VIDEOWRITER_PROP_QUALITY)<<endl;

	if (!writer.isOpened())
	{
		cerr << "Could not open the output video file for write\n";
		return;
	}

	cout << "Writing videofile: " << filename << codec << endl;

	high_resolution_clock::time_point t1 = high_resolution_clock::now();

	for (int fi = 0; fi <= lastFrame; fi++)
	{
		Mat frame;
		cap >> frame;
		if (frame.empty())
		{
			cout << fi << endl;
			cout << "Can not read video frame: " << fname << endl;
			waitKey(100000);
			return;
		}
		
		writer.write(frame);		
	}
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(t2 - t1).count();
	cout << "time single: " << duration << endl;
	writer.release();

}


void Path::WriteH264tiles(char* fname, int lastFrame, int m, int n, int codec)
{
	VideoCapture cap(fname);
	if (!cap.isOpened())
	{
		cout << "Cannot open the video file: " << fname << endl;
		waitKey(100000);
		return;

	}
	
	int frame_width = cap.get(CAP_PROP_FRAME_WIDTH);
	int frame_height = cap.get(CAP_PROP_FRAME_HEIGHT);
	int fps = cap.get(CAP_PROP_FPS);
	//int codec = VideoWriter::fourcc('X', 'V', 'I', 'D');


	vector <Mat> tileframes;


	for (int fi = 0; fi < lastFrame; fi++)
	{
		//cout << fi << endl;
		Mat frame;
		cap >> frame;
		if (frame.empty())
		{
			cout << fi << endl;
			cout << "Can not read video frame: " << fname << endl;	
			break;
		}
		tileframes.push_back(frame);
	}


	int Npx = frame_width / m;
	int Npy = frame_height / n;

	//cout << Npx<<" " << Npy << endl;
	vector < vector< Mat > > image_array;

	high_resolution_clock::time_point t1 = high_resolution_clock::now();

	for (int i = 0; i < m*n; i++)
	{
		image_array.push_back(vector<Mat>());
	}

	int m1 = 0;

	for (int iy = 0; iy < n*Npy; iy += Npy)
	{	
		for (int ix = 0; ix < m*Npx; ix +=Npx)
		{
			
			for (int fi = 0; fi < tileframes.size(); fi++)
			{
				Mat frame = tileframes[fi];
				frame = frame(Range(iy, min(frame_height,iy + Npy)), Range(ix, min(frame_width,ix+Npx)));
				image_array[m1].push_back(frame);
				//cout << m1 <<" "<<fi <<endl;
			}
			m1++;
		}
    }
	cout << "Writing videotile of: " << fname <<codec<<endl;
	for (int i = 0; i < m*n; i++)
	{
		//cout << "Writing videotile: " << i << endl;
		std::ostringstream oss;
		oss << fname<<codec<<"_" << i <<"_"<<m*n<< ".avi";
		string filename = oss.str();
		VideoWriter writer1;
		writer1.open(filename, codec, fps, Size(Npx, Npy), true);

		if (!writer1.isOpened())
		{
			cerr << "Could not open the output video file for write\n";
			return;
		}
		
		//cout << image_array[i].size() <<" "<<image_array.size()<<endl;
		for (int fi = 0; fi < tileframes.size(); fi++)
	    {
		   // cout<<i<<" " << fi << endl;
			writer1.write(image_array[i][fi]);
		}
		writer1.release();
	}

	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(t2 - t1).count();
	cout << "time tiles for:" <<fname<<": "<< duration << endl;

	return;	
		
}


void Path::DrawBoundinigBox(char* fname, int lastFrame)
{	
	VideoCapture cap(fname);
	if (!cap.isOpened())
	{
		cout << "Can't open video: " << fname << endl;
		return;
	}


	for (int fi = 1; fi < lastFrame; fi++)
	{
		
		Mat frame;
		cap >> frame;
		if (frame.empty())
		{
			cout << "Reached beyond last frame. Fi=" << fi << "; LastFrame=" << lastFrame << endl;
			break;
		}
	
		Mat dualframe;
		Mat tripleframe;
		hconcat(frame, frame, dualframe);	
		
		ERI eri(cap.get(CAP_PROP_FRAME_WIDTH), cap.get(CAP_PROP_FRAME_HEIGHT));
		ERI eritriple(tripleframe.cols, tripleframe.rows);		
		int pixelI, pixelJ = 0;
		int PxL = eri.w;
		int PxR = 0;
		int PxU = eri.h;
		int PxD = 0;
		int segi = 0;
		int fps = cap.get(CAP_PROP_FPS);
		segi = GetCamIndex(fi, fps, segi);
		cams[segi].Pan(180);
		

		for (int v = 0; v < cams[segi].h; v++)
		{
			int u = 0;
			eri.EachPixelConv2ERI(cams[segi], u, v, pixelI, pixelJ);
			if (pixelJ < PxL)
				PxL = pixelJ;
			
			u = cams[segi].w - 1;
			eri.EachPixelConv2ERI(cams[segi], u, v, pixelI, pixelJ);
			if (pixelJ > PxR)
				PxR = pixelJ;			

		}


		for (int u = 0; u < cams[segi].w; u++)
		{
			int v = 0;
			eri.EachPixelConv2ERI(cams[segi], u, v, pixelI, pixelJ);
			if (pixelI < PxU)
				PxU = pixelI;

			v = cams[segi].h - 1;
			eri.EachPixelConv2ERI(cams[segi], u, v, pixelI, pixelJ);
			if (pixelI > PxD)
				PxD = pixelI;

		}
		int half;
		int midPx;
		int ret=eri.ERIPixelInsidePPC((int)(PxU + PxD) / 2,(int)(PxL+PxR)/2, &cams[segi]);
		
		int pxudmn = min(PxU, PxD);
		int pxudmx = max(PxU, PxD);
		Vec3b insidecolor(255, 0, 0);
		//cout << PxL << " " << PxR << " " << PxU << " " << PxD <<" "<<pxudmn<<" "<<pxudmx<< endl;;

		if (ret)
		{
			midPx = eri.w + (PxR + PxL) / 2;
			
			for (int u = PxL; u < PxR; u++)
			{
				for (int v = pxudmn; v <pxudmx; v++)
				{					
					dualframe.at<Vec3b>(v, u) = insidecolor;
					frame.at<Vec3b>(v, u) = insidecolor;
				}
			}
		}

		if (!ret)
		{
			
			midPx = eri.w + (eri.w + PxR + PxL) / 2;
			for (int u = max(PxR,PxL); u < eri.w; u++)
			{
				for (int v = pxudmn; v < pxudmx; v++)
				{					
					dualframe.at<Vec3b>(v, u) = insidecolor;
					frame.at<Vec3b>(v, u) = insidecolor;
				}

			}
			for (int u = 0; u < min(PxL,PxR); u++)
			{
				for (int v =pxudmn; v < pxudmx; v++)
				{					
					dualframe.at<Vec3b>(v, u + eri.w) = insidecolor;
					frame.at<Vec3b>(v, u) = insidecolor;
				}

			}
		}

		
		Mat midcorrectedmat;
		Mat mat1;
		Mat mat2;

		hconcat(frame, dualframe, tripleframe);;
		half = eri.w / 2;		
		mat1 = tripleframe.colRange((midPx - half), midPx);
		mat2 = tripleframe.colRange(midPx, (midPx + half));
		hconcat(mat1, mat2, midcorrectedmat);
		

		//eri.VisualizeNeededPixels(frame, &cams[segi]);
		namedWindow("sample", WINDOW_NORMAL);
		resizeWindow("sample", 600, 400);
		imshow("sample", frame);
		waitKey(1000);
		//imshow("sample", dualframe);
		//waitKey(1000);
		imshow("sample", midcorrectedmat);
		waitKey(1000);


	}//end of fi loop

}


//take a image and created a distorted image from it based on compression factor
//reference Dr. Popescu GPC paper.
void Path::GetDistoredERI(PPC camera1, int compressionfactor)
{

	Mat frame;
	upload_image("./Image/source_image.PNG", frame);
	
	if (frame.empty())
	{
		cout << "Error loading image"<< endl;
		
	}

	Mat dualframe;
	Mat tripleframe;
	hconcat(frame, frame, dualframe);

	ERI eri(frame.cols, frame.rows);
	ERI eritriple(tripleframe.cols, tripleframe.rows);
	int pixelI, pixelJ = 0;
	int PxL = eri.w;
	int PxR = 0;
	int PxU = eri.h;
	int PxD = 0;
	int segi = 0;
	//int fps = cap.get(CAP_PROP_FPS);
	//segi = GetCamIndex(fi, fps, segi);
	camera1.Pan(50);
	
	/****************Get left right most points of the convectional image from the ERI and the camera***************/

	for (int v = 0; v < camera1.h; v++)
	{
		int u = 0;
		eri.EachPixelConv2ERI(camera1, u, v, pixelI, pixelJ);
		if (pixelJ < PxL)
			PxL = pixelJ;

		u = camera1.w - 1;
		eri.EachPixelConv2ERI(camera1, u, v, pixelI, pixelJ);
		if (pixelJ > PxR)
			PxR = pixelJ;

	}


	for (int u = 0; u < camera1.w; u++)
	{
		int v = 0;
		eri.EachPixelConv2ERI(camera1, u, v, pixelI, pixelJ);
		if (pixelI < PxU)
			PxU = pixelI;

		v = camera1.h - 1;
		eri.EachPixelConv2ERI(camera1, u, v, pixelI, pixelJ);
		if (pixelI > PxD)
			PxD = pixelI;

	}
	int half;
	int midPx;
	int boundingboxlength;
	int ret = eri.ERIPixelInsidePPC((int)(PxU + PxD) / 2, (int)(PxL + PxR) / 2, &camera1);

	int pxudmn = min(PxU, PxD);
	int pxudmx = max(PxU, PxD);
	Vec3b insidecolor(255, 0, 0);
	Vec3b insidecolor1(255, 255, 0);
	Vec3b insidecolor2(255, 255, 255);
	Vec3b insidecolor3(255, 0, 255);	//cout << PxL << " " << PxR << " " << PxU << " " << PxD <<" "<<pxudmn<<" "<<pxudmx<< endl;;

	/********************Get middle point and create create bounding box*******************/
	/*****************Ret is used to check weather the image has right before left (In case of image splited between end and start of the ERI***********************/

	if (ret)
	{
		midPx = eri.w + (PxR + PxL) / 2;
		boundingboxlength = PxR - PxL;		
	}

	if (!ret)
	{
		boundingboxlength = eri.w+PxR - PxL;
		midPx = eri.w + (eri.w + PxR + PxL) / 2;		
	}

	half = eri.w / 2;
	Mat midcorrectedmat;
	Mat mat1;
	Mat mat2;
	float R0x = (eri.w/2 - boundingboxlength/2);
	float R0y=pxudmn;
	int Q0x = 0;
	int Q0y = eri.h;
	int R0R1 = pxudmx - pxudmn;
	int R0R4 = boundingboxlength;	
	float We = (eri.w - boundingboxlength) / (2 * compressionfactor);
	float Het =R0y/compressionfactor;
	float Heb = (eri.h - (R0y + R0R1))/compressionfactor;

	hconcat(frame, dualframe, tripleframe);;
	
	mat1 = tripleframe.colRange((midPx - half), midPx);
	mat2 = tripleframe.colRange(midPx, (midPx + half));
	hconcat(mat1, mat2, midcorrectedmat);          ///Rotates ERI to put bounding box midle of the frame.
	
	
	Mat distortedframemat= Mat::zeros((Het+R0R1+Heb), (2*We+R0R4) , frame.type());
	Mat tmp = midcorrectedmat(Rect(R0x, R0y, R0R4, R0R1));

	/*********************** create a red outline********************************************/
	Rect RectangleToDraw(0, 0, tmp.cols, tmp.rows);
	rectangle(tmp, RectangleToDraw.tl(), RectangleToDraw.br(), Scalar(0, 0, 255), 2, 8, 0);
	
	tmp.copyTo(distortedframemat(Rect(We, Het, R0R4, R0R1)));     //copy the bounding box and paste in the distortedframemat image. 

	/*
	
	
		Q(0,0)
			___________________________________________________________________________________________________
			|\												/ \													|	
			|	\	C										 |(R0y)												|
			|		\		P()								 |													|
			|		:	\	 ________________________________|_______________________________					|
			|		:		|\								 |	   / \						|					|
			|		:		|	\	M						 |		|Het					|					|
			|		:		|		\						 |		|						|					|
			|		:		|		.	\	R0(R0x,R0y)		 |		|	   R4(R0x+R4R1,R0y)    					|					|
			|		:    	|		.		\_______________\_/____\_/_____										|					|
			|	<------(R0x)-------------->	|								|				|					|
			|		:		|		.		|								|				|					|
			|		:		|<........We..>	|								|				|					|
			|		:       |		.	    |								|				|					|
			D(Dx,Dy):<------cfactor*d--->/	|								|				|					|
			|		:		|		./		|								|				|					|
			|		:		|	/	.<--d-->(R0x, Roy+R0R1)	     			|				|					|
			|		:	  /	|		.	 R1 |______________________________	|									|					|
			|		:/		|		.		/												|					|
			|		:		|		.	/													|					|
			|		:		|		/														|					|
			|		:		|	/	N														|					|
			|		:		|/______________________________________________________________|					|
			|		:																							|
			|		:	/																						|
			|		/E																							|
			|	/																								|
			|/__________________________________________________________________________________________________|
			
	
	
	Q: original midcorreccted ERI image
	P: distorted (compressed) ERI image
	R: bounding box of the visualized ERI pixels
	R0x=horizontal distance between original ERI and bounding box
	R0y= same for vertical axis
	We= thickness of padding in x axis (x distance between P and R)
	Het=thickness of upper region
	Heb=thickness of lower region
	MN=line being transformed
	EC=line which MN will be transfered
	
	
	
	*/

/*****************Encode each of the four region: left, top, right, bottom********************/


	for (int col = 1; col < We; col++)	
	{
		for (int row = 1; row < distortedframemat.rows; row++)
		{
			float xx = (float)col / (float)row;
			float yy = (float)We / (float)Het;
			if (xx < yy && col < (float)(We*(float)(distortedframemat.rows - row) / (float)(Heb)))
			{
				//region L this one using the technique of using distance to find a point with a slope m//
				
				float Ay = col * Het / We;  //ady==Ay		 //find top point on the line (Ad)								
				float By = distortedframemat.rows - Heb * col / We;   //bottom point

				//R0Racorresponding point in distorteed				
				float R0dy = Het;					
				float R1dy = distortedframemat.rows - Heb;

				//end point of distoreted				
				
				float Md1y = distortedframemat.rows;

				float Rpy=((row - Ay)*(R1dy - R0dy)) / (By - Ay) + R0dy;
				float Rpx = We;

				float m = (Rpy - row) / (Rpx - col);
				float distance = sqrt((Rpy - row)*(Rpy - row) + (Rpx - col)*(Rpx - col));
				
				int Roriginaly = (R0y-Het)+Rpy- m*distance * compressionfactor*sqrt(1/(1+m*m));
				int Roriginalx = (R0x-We)+Rpx - distance * compressionfactor*sqrt(1 / (1 + m * m));				
				distortedframemat.at<Vec3b>(row, col) = midcorrectedmat.at<Vec3b>(Roriginaly,Roriginalx);

			}
		}
	}


	
	for (int row = 1; row < Het; row++)
	{
		for (int col = 1; col < distortedframemat.cols; col++)
		{			
			float xx = (float)col / (float)row;
			float yy = (float)We / (float)Het;
			if (xx > yy && row <= (float)(Het*(float)(distortedframemat.cols-col) / (float)(We)))
			{
				float d = Het-row;  //get distance between current line with the base line
				float Dy = R0y - compressionfactor * d;  //Get Dy by multiplying d with factor, this line in orig ERI represent line d of distorted ERI
				float Cx = Dy*(float)R0x / ((float)R0y);  //Cy==Dy

				//use equation to find E,C, M,N. We know their x distance, put that in line equation to find y distance//

				float Mx=((R0y-Het)+ row) * (float)R0x / ((float)R0y);
				float Nx= midcorrectedmat.cols + ((R0y - Het) + row) * (float)(R0x + R0R4 - midcorrectedmat.cols) / (float)R0y;
				float Ex = midcorrectedmat.cols + Dy*(float)(R0x + R0R4 - midcorrectedmat.cols) /(float) R0y;
				float Dx = Cx + (float)(Ex - Cx)*(float)(col+R0x-We-Mx) / (float)(Nx-Mx);		
				distortedframemat.at<Vec3b>(row, col) = midcorrectedmat.at<Vec3b>(Dy, Dx);

			}
		}
	}

	for (int col = distortedframemat.cols - We; col < distortedframemat.cols; col++)	
	{
		for (int row = 1; row < distortedframemat.rows; row++)		
		{
			float xx=Heb*((float)(col-distortedframemat.cols)/(float)We)+distortedframemat.rows;

			if((row<xx) && (row > (float)(Het*(float)(distortedframemat.cols - col) / (float)(We))))
			{
				//3rd region

				float d = col - distortedframemat.cols + We;
				float Dx = R0x + R0R4 + compressionfactor * d;
				float Cy = R0y * (float)(midcorrectedmat.cols - Dx) / (float)(midcorrectedmat.cols - R0x - R0R4);
				float My= R0y * (float)(midcorrectedmat.cols - (R0x-We+col)) / (float)(midcorrectedmat.cols - R0x - R0R4);
				float Ey = midcorrectedmat.rows + (Dx - midcorrectedmat.cols)*(float)(midcorrectedmat.rows - R0y - R0R1) / (float)(midcorrectedmat.cols - R0x - R0R4);
				float Ny= midcorrectedmat.rows + ((R0x - We + col) - midcorrectedmat.cols)*(float)(midcorrectedmat.rows - R0y - R0R1) / (float)(midcorrectedmat.cols - R0x - R0R4);
				float Dy = Cy + (Ey - Cy)*(float)(row+R0y-Het-My) / (float)(Ny-My);				
				distortedframemat.at<Vec3b>(row, col) = midcorrectedmat.at<Vec3b>(Dy, Dx);


			}
		}
	}

	for (int row = distortedframemat.rows-Heb; row < distortedframemat.rows; row++)
	{
		for (int col = 1; col < distortedframemat.cols; col++)
		{
			float xx = Heb * ((float)(col - distortedframemat.cols) / (float)We) + distortedframemat.rows;
			if (row > xx && col > (float)(We*(float)(distortedframemat.rows - row) / (float)(Heb)))
			{
			//4th region				
				float d = row-(distortedframemat.rows - Heb);
				float Dy = (R0y + R0R1) + compressionfactor * d;
				float Cx =  R0x * ((float)(Dy -midcorrectedmat.rows) /(float)(R0y + R0R1 - midcorrectedmat.rows));
				float Mx= R0x * ((float)(row+R0y-Het - midcorrectedmat.rows) / (float)(R0y + R0R1 - midcorrectedmat.rows));
				float Nx= midcorrectedmat.cols + (midcorrectedmat.rows - (row + R0y - Het))*((float)(midcorrectedmat.cols - R0x - R0R4) / (float)(R0y + R0R1 - midcorrectedmat.rows));
				float Ex = midcorrectedmat.cols+ (midcorrectedmat.rows- Dy)*((float)(midcorrectedmat.cols - R0x - R0R4) / (float)(R0y + R0R1 - midcorrectedmat.rows));
				float Dx = Cx + (float)(Ex - Cx)*(float)(col+R0x-We-Mx) / (float)(Nx-Mx);				
				distortedframemat.at<Vec3b>(row, col) = midcorrectedmat.at<Vec3b>(Dy, Dx);
			}
		}
	}

	float overallcompressionfactor =100* distortedframemat.rows*distortedframemat.cols / (midcorrectedmat.rows*midcorrectedmat.cols);
	cout << overallcompressionfactor << endl;
	//OverlayImage(&midcorrectedmat, &distortedframemat, Point((R0x-We), (R0y-Het)));
	//eri.VisualizeNeededPixels(frame, &cams[segi]);
	//namedWindow("sample", WINDOW_NORMAL);
	//resizeWindow("sample", 800, 400);
	//imshow("sample", frame);
	//imshow("sample", distortedframemat);
	//waitKey(10000);
	//imshow("sample", dualframe);
	//waitKey(1000);
	//imshow("sample", midcorrectedmat);
	//waitKey(10000);
	Decode(distortedframemat, eri.w, eri.h, We, Het, Heb, R0x, R0y, compressionfactor);

}//mainloop of Distorted ERI


void Path::Decode(Mat encoded_image, int original_w, int original_h,int We, int Het, int Heb, int R0x, int R0y, int compressionfactor)
{
	int R0R4 = encoded_image.cols - 2 * We;
	int R0R1 = encoded_image.rows - Het - Heb;
	Mat decodedframe(original_h, original_w, encoded_image.type());	
	Mat tmp = encoded_image(Rect(We, Het, R0R4, R0R1));

	/*********************** create a red outline********************************************/
	Rect RectangleToDraw(0, 0, tmp.cols, tmp.rows);
	rectangle(tmp, RectangleToDraw.tl(), RectangleToDraw.br(), Scalar(0, 0, 255), 2, 8, 0);
	tmp.copyTo(decodedframe(Rect(R0x, R0y, R0R4, R0R1)));     //copy the bounding box and paste in the distortedframemat image. 
	int mxrow = decodedframe.rows - 1;
	int mxcol = decodedframe.cols - 1;

	/***************************Region 01: left *****************************************/
	for (int col = 1; col < R0x; col++)
	{for (int row = 1; row < decodedframe.rows; row++)
		{
			Vec3b insidecolor(255, 0, 0);
			
			float x1 = col *  (float)R0y/(float)R0x;
			float x2 = mxrow- (float)col*(mxrow - (R0y + R0R1))/(float)R0x;
			
				if ((row > x1) && (row < x2))
				{// decodedframe.at<Vec3b>(row, col) = insidecolor;
					float d = R0x - col;
					float dx = col;
					float dy = row;
					float Dx = R0x -d/compressionfactor;
					float Cx = Dx;
					float Ex = Dx;
					float My = dx*(float)R0y / (float)R0x;
					float Cy =  Cx*(float)R0y / (float)R0x;
					float Ny = mxrow - (float)dx*(mxrow - R0y - R0R1) / (float)R0x;
					float Ey = mxrow - (float)Ex*(mxrow - R0y - R0R1) / (float)R0x;
					float Dy = Ey - (float)((Ey-Cy)*(Ny-dy)) /(float)(Ny-My);
					int distx = Dx - (R0x - We);
					int disty=Dy - (R0y - Het);
					if (disty >= encoded_image.rows)
					{
						cout << distx << " " << disty << " " << Ny << endl;
						disty = encoded_image.rows - 1;
					}
					decodedframe.at<Vec3b>(row, col) = encoded_image.at<Vec3b>(disty,distx );
				}
		}
	}  //end region 1


	//region 3


	for (int col = mxcol-R0x; col < mxcol; col++)
	{
		for (int row = 1; row < mxrow; row++)
		{
			Vec3b insidecolor(255, 0, 0);

			float y1 = (float)R0y*(mxcol-col) /(float)(mxcol-R0x-R0R4);
			float y2 = mxrow + (float)(col-mxcol)*(mxrow - (R0y + R0R1)) / (float)(mxcol-(R0x+R0R4));

			if ((row >= y1) && (row <= y2+1))
			{
				float d = col+R0x-mxcol;
				float dx = col;
				float dy = row;
				float Dx = d / compressionfactor+mxcol - R0x;;
				float Cx = Dx;
				float Ex = Dx;
				float My = (float)R0y*(mxcol - dx) / (float)(mxcol - R0x - R0R4);
				float Cy = (float)R0y*(mxcol- Dx) / (float)(mxcol - R0x - R0R4);
				float Ny = mxrow + (float)(dx - mxcol)*(mxrow - (R0y + R0R1)) / (float)(mxcol - (R0x + R0R4));
				float Ey = mxrow + (float)(Dx - mxcol)*(mxrow - (R0y + R0R1)) / (float)(mxcol - (R0x + R0R4));
				float Dy = Ey - (float)((Ey - Cy)*(Ny - dy)) / (float)(Ny - My);
				int distx = Dx - (R0x - We);
				int disty = Dy - (R0y - Het);				
				decodedframe.at<Vec3b>(row, col) = encoded_image.at<Vec3b>(disty, distx);
				
			}
		}
	}  //end region 3

	//region 2: top

	for (int row = 1; row < R0y; row++)
	{
		for (int col = 1; col < decodedframe.cols; col++)
		{
			Vec3b insidecolor(255, 0, 0);

			float y1 = col * (float)R0y / (float)R0x;
			float y2 = (float)R0y*(decodedframe.cols - col) / (float)(decodedframe.cols - R0x - R0R4);

			if ((row < y1) && (row <= y2))
			{
				float d = R0y-row;
				float dx = col;
				float dy = row;
				float Dy = R0y - d / compressionfactor;
				float Cy = Dy;
				float Ey = Dy;
				float Mx = dy * (float)R0x / (float)R0y;
				float Cx = Dy * (float)R0x / (float)R0y;;
				float Nx=  decodedframe.cols - dy*(float)(decodedframe.cols - R0x - R0R4)/(float)R0y;
				float Ex = decodedframe.cols - Dy * (float)(decodedframe.cols - R0x - R0R4) / (float)R0y;
				float Dx = Ex - (float)((Ex - Cx)*(Nx - dx)) / (float)(Nx - Mx);
				int distx = Dx- (R0x - We);
				int disty = Dy - (R0y - Het);
				//cout<<encoded_image.size() << distx << " " << disty<<endl;
				decodedframe.at<Vec3b>(row, col) = encoded_image.at<Vec3b>(disty, distx);
				//decodedframe.at<Vec3b>(row, col) = insidecolor;
			}
		}
	}  //end region 2

	//region 4: top

	for (int row = decodedframe.rows-R0y; row < decodedframe.rows; row++)
	{
		for (int col = 0; col <= decodedframe.cols; col++)
		{
			Vec3b insidecolor(255, 0, 0);

			float x1=(decodedframe.rows-row)*(float)R0x / (decodedframe.rows - (R0y + R0R1));
			float x2 =decodedframe.cols+(row - decodedframe.rows)*(float)(decodedframe.cols - (R0x + R0R4))/(decodedframe.rows - (R0y + R0R1));

			if ((col >= x1-2) && (col <= x2))
			{
				float d = row-(R0y+R0R1);
				float dx = col;
				float dy = row;
				float Dy =(float)d/(float)compressionfactor+ (R0y+R0R1);
				float Cy = Dy;
				float Ey = Dy;
				float Mx = (decodedframe.rows - dy)*(float)R0x / (decodedframe.rows - (R0y + R0R1));
				float Cx = (decodedframe.rows - Dy)*(float)R0x / (decodedframe.rows - (R0y + R0R1));
				float Nx = decodedframe.cols + (dy - decodedframe.rows)*(float)(decodedframe.cols - (R0x + R0R4)) / (decodedframe.rows - (R0y + R0R1));
				float Ex = decodedframe.cols + (Dy - decodedframe.rows)*(float)(decodedframe.cols - (R0x + R0R4)) / (decodedframe.rows - (R0y + R0R1));
				float Dx = Ex - (float)((Ex - Cx)*(Nx - dx)) / (float)(Nx - Mx);
				int distx = Dx - (R0x - We);
				int disty = Dy - (R0y - Het)-1;
				//cout<<encoded_image.size() << distx << " " << disty<<endl;
				decodedframe.at<Vec3b>(row, col) = encoded_image.at<Vec3b>(disty, distx);
				//decodedframe.at<Vec3b>(row, col) = insidecolor;
			}
		}
	}  //end region 4

	namedWindow("sample", WINDOW_NORMAL);
	resizeWindow("sample", 800, 400);	
	imshow("sample", decodedframe);
	waitKey(10000);

	/*


		Q0(0,0)
			___________________________________________________________________________________________________
			|\												/ \													|
			|	\	C										 |(R0y)												|
			|		\		P()								 |													|
			|		:	\	 ________________________________|_______________________________					|
			|		:		|\								 |	   / \						|					|
			|		:		|	\	M						 |		|Het					|					|
			|		:		|		\						 |		|						|					|
			|		:		|		.	\	R0(R0x,R0y)		 |		|	   R4(R0x+R4R1,R0y)    					|					|
			|		:    	|		.		\_______________\_/____\_/_____										|					|
			|	<------(R0x)-------------->	|								|				|					|
			|		:		|		.		|								|				|					|
			|		:		|<........We..>	|								|				|					|
			|		:       |		.	    |								|				|					|
			D(Dx,Dy):<------cfactor*d--->/	|								|				|					|
			|		:		|		./		|								|				|					|
			|		:		|	/	.<--d-->(R0x, Roy+R0R1)	     			|				|					|
			|		:	  /	|		.	 R1 |______________________________	|									|					|
			|		:/		|		.		/												|					|
			|		:		|		.	/													|					|
			|		:		|		/														|					|
			|		:		|	/	N														|					|
			|		:		|/______________________________________________________________|					|
			|		:																							|
			|		:	/																						|
			|		/E																							|
			|	/																								|
			|/__________________________________________________________________________________________________|
	Q1(0, eri.h)


	Q: original midcorreccted ERI image
	P: distorted (compressed) ERI image
	R: bounding box of the visualized ERI pixels
	R0x=horizontal distance between original ERI and bounding box
	R0y= same for vertical axis
	We= thickness of padding in x axis (x distance between P and R)
	Het=thickness of upper region
	Heb=thickness of lower region
	MN=line being transformed
	EC=line which MN will be transfered



	*/



}

/*
Overlay image: Put one image transparantely on top of other image
*/
void Path::OverlayImage(Mat* src, Mat* overlay, const Point& location)
{
	for (int y = max(location.y, 0); y < src->rows; ++y)
	{
		int fY = y - location.y;

		if (fY >= overlay->rows)
			break;

		for (int x = max(location.x, 0); x < src->cols; ++x)
		{
			int fX = x - location.x;

			if (fX >= overlay->cols)
				break;

			double opacity = ((double)overlay->data[fY * overlay->step + fX * overlay->channels() + 3]) / 255;

			for (int c = 0; opacity > 0 && c < src->channels(); ++c)
			{
				unsigned char overlayPx = overlay->data[fY * overlay->step + fX * overlay->channels() + c];
				unsigned char srcPx = src->data[y * src->step + x * src->channels() + c];
				src->data[y * src->step + src->channels() * x + c] = srcPx * (1. - opacity) + overlayPx * opacity;
			}
		}
	}
}// End Overlay image
