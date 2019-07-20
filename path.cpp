
#include"path.h"
#include"image.h"
#include<stdio.h>
#include<math.h>
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
	print("One Path created" << endl);
}

Path::~Path() {
	print("One Path deleted" << endl);
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
	//print(cams.size() - 1 << endl);	
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
		print("error: can't open file: " <<filename<< endl);
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

	print("read: " << cams.size() << " cameras from: " << filename << endl);
	
		
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
		print(fi << " " << segi << "; ");
		//imshow("outputImage", convPixels);
		eri.VisualizeNeededPixels(erivis, &(cams[segi]));
		print("done visualisation" << endl);
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
		print("done visualisation" << endl);
		waitKey(1);

	}

	//pixelcalc.PrintPixelInterval();
	pixelcalc.SaveIntervalTxt();
	print("done Interval saving" << endl);
}//most outer loop of playbackpathvideoopixelinterval




int Path::GetCamIndex(int fi, int fps, int segi) {

	int ret = segi;
	print(fps <<""<<fi<< endl);
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
			print("frame No: "<< fi << "chunk no: " << fi / (fps*t) << endl);
			for (int i = 0; i < m*n; i++)
			{
				
				if (Tilebitmap[i]==1)
				{
					totaltiles++;
				}
				print("tile: " << Tilebitmap[i] <<", Total-"<<totaltiles<< endl);
				Tilebitmap.at(i)=0;
			}
			waitKey(10);
		}

		
	}//outer for loop for all frame
	print("H: "<< eri.h<<" W: "<< eri.w<<" M: "<<m<<" N: "<<n<<" FPS: "<<fps<<" "<<" T: "<<t<<" Total Tiles: "<<totaltiles<< endl);
	double total_tile_pixel = eri.h*eri.w*fps*t*1UL / (m*n);
	total_tile_pixel = totaltiles * total_tile_pixel*1UL;
	double result = (ERItotal*lastFrame * 100UL /total_tile_pixel) ;
	//print("Total_tile_pixel: " << total_tile_pixel << endl);
	//print("Total requierd ERI pixel: " << ERItotal*lastFrame << endl);
	print("Required and supplied %: " << result << endl);

}

void Path::VDrotationAvg()
{	
	double total_angle=0;
	float angle=0;
	for (int i = 0; i < cams.size(); i++)
	{
		V3 cam1 = cams[i].GetVD();
		V3 cam2 = cams[i + 1].GetVD();
		print(cam1 <<"; " <<cam2<< endl);
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
		print("Angle: " << angle << " Total angle: " << total_angle << endl);
	
	}
	
	print("No of camera: " << cams.size() << "; Total Angle: " << total_angle << endl);
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



void Path::DrawBoundinigBoxTemp(char* fname, int lastFrame)
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

		Mat newERI(frame.rows,frame.cols, frame.type());
	
		ERI eri(cap.get(CAP_PROP_FRAME_WIDTH), cap.get(CAP_PROP_FRAME_HEIGHT));		
		int pixelI, pixelJ = 0;
		int PxL = eri.w;
		int PxR = 0;
		int PxU = eri.h;
		int PxD = 0;
		int segi = 0;
		int fps = cap.get(CAP_PROP_FPS);
		segi = GetCamIndex(fi, fps, segi);
		//cams[segi].Pan(180);
		PPC camera(110.0f, 800, 400);
		V3 pb=camera.GetVD();
		V3 pa = cams[segi].GetVD();

		// build local coordinate system of RERI
		V3 xaxis = cams[segi].a.UnitVector();
		V3 yaxis = cams[segi].b.UnitVector()*-1.0f;
		V3 zaxis = xaxis ^ yaxis;
		M33 reriCS;
		reriCS[0] = xaxis;
		reriCS[1] = yaxis;
		reriCS[2] = zaxis;

		
		RotateXYaxisERI2RERI(frame, newERI, pb,pa,reriCS);

		

		//cams[segi].PositionAndOrient(V3(0, 0, 0),pb, V3(0, 1, 0));
		cams[segi] = camera;
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
		Vec3b insidecolor(255, 0, 0);
		cout << PxL << " " << PxR << " " << PxU << " " << PxD << endl;	

		int left = newERI.cols / 2-(((PxR - PxL))/2);
		int up = newERI.rows / 2-(((PxD - PxU)) / 2);

		Rect RectangleToDraw(left, up, (PxR-PxL), (PxD-PxU));
		rectangle(newERI, RectangleToDraw.tl(), RectangleToDraw.br(), Scalar(0, 0, 255), 2, 8, 0);

				
		/*
		for (int u = PxL; u < PxR; u++)
		{
			for (int v = PxU; v < PxD; v++)
			{
				
				newERI.at<Vec3b>(v, u) = insidecolor;
			}
		}
		*/
		
		namedWindow("sample", WINDOW_NORMAL);
		resizeWindow("sample", 600, 400);
		imshow("sample", newERI);
		waitKey(10);
	}//end of fi loop

}





/*************************************************************************************************************/
/**************************************************************************************************************/
//take a image and created a distorted image from it based on compression factor
//reference Dr. Popescu GPC paper.
Mat Path::Encode(PPC camera1, int compressionfactor, Mat frame, vector<float>& We1, vector<float>& Het1, vector<float>& Heb1, vector<float>& R0x1, vector<float>& R0y1, vector<float>& R0R1_1, vector<float>& R0R4_1)
{	
	
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
	float half;
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

	half = (float)eri.w / 2.0f;
	Mat midcorrectedmat;
	Mat mat1;
	Mat mat2;
	float R0x = ((float)eri.w/2.0f - (float)boundingboxlength/2.0f);
	float R0y=pxudmn;
	int Q0x = 0;
	int Q0y = eri.h;
	int R0R1 = pxudmx - pxudmn;
	int R0R4 = boundingboxlength;	
	float We = (float)(eri.w - boundingboxlength) / (float)(2 * compressionfactor);
	float Het =(float)R0y/(float)compressionfactor;
	float Heb = (float)(eri.h - (R0y + R0R1))/(float)compressionfactor;

	hconcat(frame, dualframe, tripleframe);;
	
	mat1 = tripleframe.colRange((midPx - half), midPx);
	mat2 = tripleframe.colRange(midPx, (midPx + half));
	hconcat(mat1, mat2, midcorrectedmat);          ///Rotates ERI to put bounding box midle of the frame.
	
	print(R0R1 << " " << R0R4 << " " << R0x);
	
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
	int mxdrow = distortedframemat.rows - 1;
	int mxdcol= distortedframemat.cols - 1;
	int mxcrow = midcorrectedmat.rows - 1;
	int mxccol = midcorrectedmat.cols - 1;

	//to calculated nonuniform distance and then correct it by rotatiing the array/vector//

	/*
	In this region with increasing the array comes in oppossite direction. means when col increases
	d actually increases in inverse direction.  Thats why the 
	array needs to be inversed. For this reason we created next two loop. First one 
	calculate inverted d and second one correct it.
	f(d)=axx+bx+c; cond1: f(both_edge_of_dist_ERI)= both_edge_of_big_ERI; f'(d_inner_edge_of_dist)=1 
	to make sampling constant with the inner rectangle
	the equation becomes b=1; a=(100-10)/10*10. Where d=0 maps to 0 and d=10 maps to 100: 
	means edges are at 0,10 for distmat and 0,100 for originalmat 
	*/

	vector <float> nonuniformD;
	vector <float > nonuniformDrowcorrected;
	//to understand logic for this two loop, print them somewhere and see with 
	//increasing col quad_out keep increasing the gap. this d here is not our real d.
	//read d starts from the R0R1 and go towards QQ1. So we need to turn the array top to bottom
	for (int col = 0; col < We; col++)
	{
		float d = col;
		float quad_out = ((float)(R0x - We) / (float)(We*We))*d*d + d;
		nonuniformD.push_back(quad_out);

	}

	for (int i = nonuniformD.size()-1; i> -1; i--)
	{
		nonuniformDrowcorrected.push_back(nonuniformD[i]);
		
	}

//////////////
	for (int col = 0; col < We; col++)	
	{
		float Dx = R0x - nonuniformDrowcorrected[col];
		//print("Dx: " << Dx << endl);
		for (int row = 0; row < distortedframemat.rows; row++)
		{
			float xx = (float)col / (float)row;
			float yy = (float)We / (float)Het;
			if (xx < yy && col < (float)(We*(float)(mxdrow - row) / (float)(Heb)))
			{
				//region L this one using the technique of using distance to find a point with a slope m//
			
				//float Dx = R0x - nonuniformDrowcorrected[col];
				float Cx = Dx;
				float Ex = Dx;
				float Cy =Cx* (float)R0y / (float)R0x;
				
				//use equation to find E,C, M,N. We know their x distance, put that in line equation to find y distance//

				float My = ((R0x - We) + col) * (float)R0y / ((float)R0x);
				float Ny = mxcrow + (((R0x - We) + col) * (float)(R0y + R0R1 - mxcrow)) / (float)R0x;
				float Ey = mxcrow + (Ex * (float)(R0y + R0R1 - mxcrow)) / (float)R0x;
				float Dy = Cy + (float)(Ey - Cy)*(float)(row + R0y - Het - My) / (float)(Ny - My);
				distortedframemat.at<Vec3b>(row, col) = midcorrectedmat.at<Vec3b>(Dy, Dx);
				
				bilinearinterpolation(distortedframemat, midcorrectedmat, row, col, Dy, Dx);
			}
		}
	}
	//print("loop1" << endl);

	//same logic for quadratic equation of region one

	vector <float> nonuniformD2;
	vector <float > nonuniformDrowcorrected2;

	for (int row = 0; row < Het; row++)
	{
		float d = row;
		float quad_out = ((float)(R0y - Het) / (float)(Het*Het))*d*d + d;
		nonuniformD2.push_back(quad_out);

	}

	for (int i = nonuniformD2.size() - 1; i > -1; i--)
	{
		nonuniformDrowcorrected2.push_back(nonuniformD2[i]);
		//cout << "i:" << i << " d: " << nonuniformD[i] << endl;
	}



	for (int row = 0; row < Het; row++)
	{
		for (int col = 0; col < distortedframemat.cols; col++)
		{			
			float xx = (float)col / (float)row;
			float yy = (float)We / (float)Het;
			if (xx > yy && row < (float)(Het*(float)(mxdcol-col) / (float)(We)))
			{
				//Next two commented line for uniform sampling
				//float d = Het-row;  //get distance between current line with the base line			
				//float Dy = R0y - compressionfactor * d;  //Get Dy by multiplying d with factor, this line in orig ERI represent line d of distorted ERI
				
				float Dy = R0y - nonuniformDrowcorrected2[row];
				
				float Cx = Dy*(float)R0x / ((float)R0y);  //Cy==Dy

				//use equation to find E,C, M,N. We know their x distance, put that in line equation to find y distance//

				float Mx=((R0y-Het)+ row) * (float)R0x / ((float)R0y);
				float Nx= mxccol + ((R0y - Het) + row) * (float)(R0x + R0R4 - mxccol) / (float)R0y;
				float Ex = mxccol + Dy*(float)(R0x + R0R4 - mxccol) /(float) R0y;
				float Dx = Cx + (float)(Ex - Cx)*(float)(col+R0x-We-Mx) / (float)(Nx-Mx);		
				distortedframemat.at<Vec3b>(row, col) = midcorrectedmat.at<Vec3b>(Dy, Dx);
				//print(Dy << ","<<Dx << endl);
				bilinearinterpolation(distortedframemat, midcorrectedmat, row, col, Dy, Dx);
			}
		}
	}

	//print("loop2" << endl);
	for (int col = We+R0R4; col < distortedframemat.cols; col++)	
	{
		float d = col - We - R0R4;
		float quad_out = ((float)(R0x - We) / (float)(We*We))*d*d + d;
		//float Dx = R0x + R0R4 + compressionfactor * d;
		float Dx = R0x + R0R4 + quad_out;

		for (int row = 0; row < distortedframemat.rows; row++)		
		{
			float xx=Heb*((float)(col-mxdcol)/(float)We)+mxdrow;

			if((row<xx) && (row > (float)(Het*(float)(mxdcol - col) / (float)(We))))
			{
				//3rd region. This region is alligned with our quadratic equation means
				// when col increase d also increase in same direction. So we dont need
				// additional work we did in region 1			
			

				
				float Cy = R0y * (float)(mxccol - Dx) / (float)(mxccol - R0x - R0R4);
				float My= R0y * (float)(mxccol - (R0x-We+col)) / (float)(mxccol - R0x - R0R4);
				float Ey = mxcrow + (Dx - mxccol)*(float)(mxcrow - R0y - R0R1) / (float)(mxccol - R0x - R0R4);
				float Ny= mxcrow + ((R0x - We + col) - mxccol)*(float)(mxcrow - R0y - R0R1) / (float)(mxccol - R0x - R0R4);
				float Dy = Cy + (Ey - Cy)*(float)(row+R0y-Het-My) / (float)(Ny-My);				
				distortedframemat.at<Vec3b>(row, col) = midcorrectedmat.at<Vec3b>(Dy, Dx);
				bilinearinterpolation(distortedframemat, midcorrectedmat, row, col, Dy, Dx);
				
			}
		}
		//print("Dx2:" << Dx << endl);
	}

	//print("loop3" << endl);
	for (int row = Het+R0R1; row < distortedframemat.rows; row++)
	{
		for (int col = 0; col < distortedframemat.cols; col++)
		{
			float xx = Heb * ((float)(col - mxdcol) / (float)We) + mxdrow;
			if (row > xx && col > (float)(We*(float)(mxdrow - row) / (float)(Heb)))
			{
			//4th region has same logic as second region for quad//

				float d = row-(Het + R0R1);
				float quad_out = ((float)(midcorrectedmat.rows-R0y-R0R1 - Heb) / (float)(Heb*Heb))*d*d + d; //Should replace Roy
				float Dy = (R0y + R0R1) + quad_out;
				float Cx =  R0x * ((float)(Dy -mxcrow) /(float)(R0y + R0R1 - mxcrow));
				float Mx= R0x * ((float)(row+R0y-Het - mxcrow) / (float)(R0y + R0R1 - mxcrow));
				float Nx= mxccol + (mxcrow - (row + R0y - Het))*((float)(mxccol - R0x - R0R4) / (float)(R0y + R0R1 - mxcrow));
				float Ex = mxccol+ (mxcrow- Dy)*((float)(mxccol - R0x - R0R4) / (float)(R0y + R0R1 - mxcrow));
				float Dx = Cx + (float)(Ex - Cx)*(float)(col+R0x-We-Mx) / (float)(Nx-Mx);				
				//distortedframemat.at<Vec3b>(row, col) = midcorrectedmat.at<Vec3b>(Dy, Dx);
				bilinearinterpolation(distortedframemat, midcorrectedmat, row, col, Dy, Dx);
			}
		}
	}
	//print("loop4" << endl);
	float overallcompressionfactor =100* distortedframemat.rows*distortedframemat.cols / (midcorrectedmat.rows*midcorrectedmat.cols);

	//OverlayImage(&midcorrectedmat, &distortedframemat, Point((R0x-We), (R0y-Het)));
	//eri.VisualizeNeededPixels(frame, &cams[segi]);
	We1.push_back(We);
	Het1.push_back(Het);
	Heb1.push_back(Heb);
	R0x1.push_back(R0x);
	R0y1.push_back(R0y);
	R0R1_1.push_back(R0R1);
	R0R4_1.push_back(R0R4);
	//print(We<<","<< R0x<<","<< Het<<" "<< Heb<<" "<< R0y<<endl);
	return distortedframemat;

}//mainloop of End of Encoding








Mat Path::Decode(Mat encoded_image, int original_w, int original_h,float We, float Het, float Heb, float R0x, float R0y, float R0R1, float R0R4, int compressionfactor)
{
	//int R0R4 = encoded_image.cols - 2 * We;
	//int R0R1 = encoded_image.rows - Het - Heb;
	Mat decodedframe(original_h, original_w, encoded_image.type());	
	Mat tmp = encoded_image(Rect(We, Het, R0R4, R0R1));

	/*********************** create a red outline********************************************/
	Rect RectangleToDraw(0, 0, tmp.cols, tmp.rows);
	rectangle(tmp, RectangleToDraw.tl(), RectangleToDraw.br(), Scalar(0, 0, 255), 2, 8, 0);
	tmp.copyTo(decodedframe(Rect(R0x, R0y, R0R4, R0R1)));     //copy the bounding box and paste in the distortedframemat image. 
	int mxrow = decodedframe.rows - 1;
	int mxcol = decodedframe.cols - 1;

	/***************************Region 01: left *****************************************/
	for (int col = 0; col < R0x; col++)
	{for (int row = 0; row < decodedframe.rows; row++)
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
					float distx = Dx - (R0x - We);
					float disty=Dy - (R0y - Het);
					
					if (disty >= encoded_image.rows)
					{
						//cout << distx << " " << disty << " " << encoded_image.size() << endl;
						disty = encoded_image.rows - 1;
					}
					if (distx >= encoded_image.cols)
					{
						//cout << distx << " " << disty << " " << encoded_image.size() << endl;
						distx = encoded_image.cols - 1;
					}
					if (disty < 0)
					{
						//cout << distx << " " << disty << " " << encoded_image.size() << endl;
						disty = 0;
					}
					if (distx < 0)
					{
						//cout << distx << " " << disty << " " << encoded_image.size() << endl;
						distx =0;
					}
					//cout << distx << " " << disty << endl;
					//decodedframe.at<Vec3b>(row, col) = encoded_image.at<Vec3b>(disty,distx );
					bilinearinterpolation(decodedframe, encoded_image, row, col, disty, distx);
				}
		}
	}  //end region 1

	//print("dloop1" << endl);
	//region 3


	for (int col = R0x+R0R4; col < mxcol; col++)
	{
		for (int row = 0; row < mxrow; row++)
		{
			Vec3b insidecolor(255, 0, 0);

			float y1 = (float)R0y*(mxcol-col) /(float)(mxcol-R0x-R0R4);
			float y2 = mxrow + (float)(col-mxcol)*(mxrow - (R0y + R0R1)) / (float)(mxcol-(R0x+R0R4));

			if ((row > y1) && (row < y2))
			{
				float d = col-(R0x + R0R4);
				float dx = col;
				float dy = row;
				float Dx = d / compressionfactor+ (R0x + R0R4);;
				float Cx = Dx;
				float Ex = Dx;
				float My = (float)R0y*(mxcol - dx) / (float)(mxcol - R0x - R0R4);
				float Cy = (float)R0y*(mxcol- Dx) / (float)(mxcol - R0x - R0R4);
				float Ny = mxrow + (float)(dx - mxcol)*(mxrow - (R0y + R0R1)) / (float)(mxcol - (R0x + R0R4));
				float Ey = mxrow + (float)(Dx - mxcol)*(mxrow - (R0y + R0R1)) / (float)(mxcol - (R0x + R0R4));
				float Dy = Ey - (float)((Ey - Cy)*(Ny - dy)) / (float)(Ny - My);
				float distx = Dx - (R0x - We);
				float disty = Dy - (R0y - Het);
				if (disty >= encoded_image.rows)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					disty = encoded_image.rows - 1;
				}
				if (distx >= encoded_image.cols)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					distx = encoded_image.cols- 1;
				}
				if (disty < 0)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					disty = 0;
				}
				if (distx < 0)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					distx = 0;
				}
				//print(distx << " " << disty << " " << encoded_image.size() << endl);
				//decodedframe.at<Vec3b>(row, col) = encoded_image.at<Vec3b>(disty, distx);
				bilinearinterpolation(decodedframe, encoded_image, row, col, disty, distx);
				
			}
		}
	}  //end region 3


	//print("dloop2" << endl);

	//region 2: top

	for (int row = 0; row < R0y; row++)
	{
		for (int col = 0; col < decodedframe.cols; col++)
		{
			Vec3b insidecolor(255, 0, 0);

			float y1 = col * (float)R0y / (float)R0x;
			float y2 = (float)R0y*(mxcol - col) / (float)(mxcol - R0x - R0R4);

			if ((row < y1) && (row < y2))
			{
				float d = R0y-row;
				float dx = col;
				float dy = row;
				float Dy = R0y - d / compressionfactor;
				float Cy = Dy;
				float Ey = Dy;
				float Mx = dy * (float)R0x / (float)R0y;
				float Cx = Dy * (float)R0x / (float)R0y;;
				float Nx=  mxcol - dy*(float)(mxcol - R0x - R0R4)/(float)R0y;
				float Ex = mxcol - Dy * (float)(mxcol - R0x - R0R4) / (float)R0y;
				float Dx = Ex - (float)((Ex - Cx)*(Nx - dx)) / (float)(Nx - Mx);
				float distx = Dx- (R0x - We);
				float disty = Dy - (R0y - Het);

				if (disty > encoded_image.rows-1)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					disty = encoded_image.rows - 1;
				}

				if (distx > encoded_image.cols-1)
				{
					//print(" danger1 " << distx << endl);
					//print(distx << " " << disty << " " << encoded_image.size() << endl);
					distx = encoded_image.cols - 1;
				}
				if (disty < 0)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					disty = 0;
				}
				if (distx < 0)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					distx = 0;
				}
				
				//decodedframe.at<Vec3b>(row, col) = encoded_image.at<Vec3b>(disty, distx);
				
				bilinearinterpolation(decodedframe, encoded_image, row, col, disty, distx);
			}
		}
	}  //end region 2

	//print("dloop3" << endl);
	//region 4: bottom
	
	for (int row = R0y+R0R1; row < decodedframe.rows; row++)
	{
		for (int col =0; col < decodedframe.cols; col++)
		{
			Vec3b insidecolor(255, 0, 0);

			float x1=(mxrow-row)*(float)R0x / (mxrow - (R0y + R0R1));
			float x2 =mxcol+(row - mxrow)*(float)(mxcol - (R0x + R0R4))/(mxrow - (R0y + R0R1));

			if ((col > x1) && (col < x2))
			{
				float d = row-(R0y + R0R1);
				float dx = col;
				float dy = row;
				float Dy =(float)d/(float)compressionfactor+ (R0y + R0R1);
				float Cy = Dy;
				float Ey = Dy;
				float Mx = (mxrow - dy)*(float)R0x / (mxrow - (R0y + R0R1));
				float Cx = (mxrow - Dy)*(float)R0x / (mxrow - (R0y + R0R1));
				float Nx = mxcol + (dy - mxrow)*(float)(mxcol - (R0x + R0R4)) / (mxrow - (R0y + R0R1));
				float Ex = mxcol + (Dy - mxrow)*(float)(mxcol - (R0x + R0R4)) / (mxrow - (R0y + R0R1));
				float Dx = Ex - (float)((Ex - Cx)*(Nx - dx)) / (float)(Nx - Mx);
				float distx = Dx - (R0x - We);
				float disty = Dy - (R0y - Het)-1;
				if (disty >= encoded_image.rows)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					disty = encoded_image.rows - 1;
				}
				if (distx >= encoded_image.cols)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					distx = encoded_image.cols - 1;
				}
				if (disty <0)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					disty =0;
				}
				if (distx <0)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					distx =0;
				}
				//decodedframe.at<Vec3b>(row, col) = encoded_image.at<Vec3b>(disty, distx);
				bilinearinterpolation(decodedframe, encoded_image, row, col, disty, distx);
			}
		}
	}  //end region 4
	//print("dloop4" << endl);
	return decodedframe;

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


//video encode function*************************************************************************/

vector<Mat> Path::videoencode(char* fname, int lastFrame, vector<float>& We, vector<float>& Het, vector<float>& Heb, vector<float>& R0x, vector<float>& R0y, vector<float>& R0R1, vector<float>& R0R4, int compressionfactor)
{
	VideoCapture cap(fname);
	if (!cap.isOpened())
	{
		cout << "Cannot open the video file: " << fname << endl;
		waitKey(100000);
		return {};
	}
	
	
	int fps = 30;
	int segi = 0;
	float max_w = 0;
	float max_h = 0;

	vector <Mat> retbuffer;

	namedWindow("sample", WINDOW_NORMAL);
	resizeWindow("sample", 800, 800);

	for (int fi = 0; fi < lastFrame; fi++)
	{		
		Mat frame;
		cap >> frame;
		if (frame.empty())
		{
			cout << fi << endl;
			cout << "Can not read video frame: " << fname << endl;
			break;
		}
		
		
		Mat ret;
		segi = GetCamIndex(fi, fps, segi);		
		ret=Encode(cams[segi], 5, frame, We, Het, Heb, R0x, R0y, R0R1, R0R4);	
		if (ret.cols > max_w)
		{
			max_w = ret.cols;
		}
		if (ret.rows > max_h)
		{
			max_h = ret.rows;
		}
		retbuffer.push_back(ret);		
	}

	cap.release();
	
	VideoWriter writer;
	int codec = VideoWriter::fourcc('H', '2', '6', '4');
	writer.set(VIDEOWRITER_PROP_QUALITY, 80);
	string filename = "./Video/encodingtest/rollerh264encod.MKV";
	cout << "Writing videofile: " << filename << codec << endl;
	writer.open(filename, codec, fps, Size(max_w, max_h), true);

	if (!writer.isOpened())
	{
		cerr << "Could not open the output video file for write\n";
		return {};
	}

	for (int i = 0; i < retbuffer.size(); i++)
	{
		Mat temp1;
		Mat ret;
		ret = retbuffer[i];
		
		//cout << ret.size()<<"," <<max_w<<","<<max_h<< endl;
		if (max_w>ret.cols)
		{
			
			temp1 = Mat::zeros(max_h, max_w, ret.type());
			ret.copyTo(temp1(Rect(0, 0, ret.cols, ret.rows)));
			
		//imshow("sample", ret);
		//waitKey(1000);
			writer.write(temp1);

		}
		else 
		{ 
			writer.write(ret); 
		}
		
	}

	writer.release();
	return retbuffer; 
}

void Path::videodecode(char* fname, int lastFrame, int original_w, int original_h, vector<float>& We, vector<float>& Het, vector<float>& Heb, vector<float>& R0x, vector<float>& R0y, vector<float>& R0R1, vector<float>& R0R4, int compressionfactor)
{
	VideoCapture cap(fname);
	if (!cap.isOpened()) {
		cout << "Cannot open the video file: " << fname << endl;
		waitKey(100000);
		return;

	}
	Path path1;
	Mat retdecode;

	namedWindow("sample", WINDOW_NORMAL);
	resizeWindow("sample", 800, 800);


	VideoWriter writer;
	int codec = VideoWriter::fourcc('H', '2', '6', '4');
	writer.set(VIDEOWRITER_PROP_QUALITY, 80);
	int fps = 30;
	string filename = "./Video/encodingtest/rollerh264decod.avi";
	writer.open(filename, codec, fps, Size(original_w, original_h), true);

	cout << "Writing videofile: " << filename << codec << endl;

	for (int fi = 0; fi < lastFrame; fi++)
	{
		Mat frame;
		cap >> frame;
		cout << fi <<endl;
		if (frame.empty())
		{
			cout << "Can not read video frame: " << fname << endl;
			waitKey(100000);
			return;
		}

		imshow("sample", frame);
		waitKey(30);

		retdecode = path1.Decode(frame,original_w, original_h, We[fi], Het[fi], Heb[fi], R0x[fi], R0y[fi], R0R1[fi], R0R4[fi], compressionfactor);
		writer.write(retdecode);
	
	}
	writer.release();
}


void Path::bilinearinterpolation(Mat &output, Mat &source, int Orow, int Ocol, float uf, float vf )
{
	

	if (vf < (source.cols - 1.5)&& vf> 0.5 &&  uf> 0.5 && uf < (source.rows - 1.5))
	{

		int u0 = (int)(uf - 0.5f);
		int v0 = (int)(vf - 0.5f);
		float dx = vf - 0.5f - v0;
		float dy = uf - 0.5f - u0;

		Vec3b color = (1 - dx)*(1 - dy)* source.at<Vec3b>(u0, v0) + (dx)*(1 - dy)*source.at<Vec3b>(u0, v0 + 1) +
			dx * dy*source.at<Vec3b>(u0 + 1, v0 + 1) + (1 - dx)*dy*source.at<Vec3b>(u0 + 1, v0);

		output.at<Vec3b>(Orow, Ocol) = color;
	}
	else
	{
		
		if (vf >= source.cols)
		{
			vf = source.cols - 1;
			//print(vf << " " << uf << " " << source.size() << endl);
		}
		if (uf >= source.rows)
		{
			uf = source.rows - 1;
			//print(vf << " " << uf << " " << source.size() << endl);
		}
		if (vf <0)
		{
			vf = 0;
			//print(vf << " " << uf << " " << source.size() << endl);
		}
		if (uf <0)
		{
			uf = 0;
			//print(vf << " " << uf << " " << source.size() << endl);
		}
		output.at<Vec3b>(Orow, Ocol) = source.at<Vec3b>((int)uf, (int)vf);
	}
}

void Path:: PixelXMapERI2RERI(int ur, int uc, int w, float &u)
{
    u = ur + (uc - w / 2);
	u = (u < 0) ? (w + u) : u; // wrap around ERI if needed (left)
	u = (u > w) ? (u - w) : u; // wrap around ERI if needed (right)
}

void Path::RotateXaxisERI2RERI(Mat originERI, Mat& newERI, PPC camera1)
{
	ERI oERI(originERI.cols, originERI.rows);
	V3 p = camera1.GetVD();
	int uc = oERI.Lon2PixJ(oERI.GetXYZ2Longitude(p));
	float u = 0;
	for (int j = 0; j < newERI.rows; j++)
	{
		for (int i = 0; i < newERI.cols; i++)
		{
			PixelXMapERI2RERI(i, uc, newERI.cols, u);
			newERI.at<Vec3b>(j, i) = originERI.at<Vec3b>(j, u);
		}
	}
	
}

void Path::BuildERI2RERI(Mat originERI,  PPC camera1)
{
	Mat newERI = Mat::zeros(originERI.rows, originERI.cols, originERI.type());	

	
	namedWindow("sample", WINDOW_NORMAL);
	resizeWindow("sample", 800, 400);
	
	// build local coordinate system of RERI
	V3 xaxis = camera1.a.UnitVector();
	V3 yaxis = camera1.b.UnitVector()*-1.0f;
	V3 zaxis = xaxis ^ yaxis;
	M33 reriCS;
	reriCS[0] = xaxis;
	reriCS[1] = yaxis;
	reriCS[2] = zaxis;


	V3 p = camera1.GetVD();
	camera1.Pan(0);
	camera1.Tilt(45.0f);
	V3 p1 = camera1.GetVD();
	RotateXYaxisERI2RERI(originERI, newERI, p, p1, reriCS);

	imshow("sample", newERI);
	waitKey(100000);


}


void Path::BuildERI2RERIVideo(Mat originERI, PPC ReferenceCamera)
{

	VideoCapture cap("./Video/roller.MKV");
	if (!cap.isOpened())
	{
		cout << "Cannot open the video file: "<< endl;
		waitKey(100000);
		return;
	}


	int fps = 30;
	int segi = 0;
	float max_w = 0;
	float max_h = 0;

	
	for (int fi = 0; fi < 100; fi++)
	{
		Mat frame;
		cap >> frame;
		if (frame.empty())
		{
			cout << fi << endl;
			cout << "Can not read video frame: " << endl;
			break;
		}

		Mat newERI = Mat::zeros(frame.rows, frame.cols, frame.type());

		segi = GetCamIndex(fi, fps, segi);

		V3 p = ReferenceCamera.GetVD();
		V3 p1 = cams[segi].GetVD();

		// build local coordinate system of RERI
		V3 xaxis = cams[segi].a.UnitVector();
		V3 yaxis = cams[segi].b.UnitVector()*-1.0f;
		V3 zaxis = xaxis ^ yaxis;
		M33 reriCS;
		reriCS[0] = xaxis;
		reriCS[1] = yaxis;
		reriCS[2] = zaxis;


		RotateXYaxisERI2RERI(frame, newERI, p, p1, reriCS);


		ERI eri(cap.get(CAP_PROP_FRAME_WIDTH), cap.get(CAP_PROP_FRAME_HEIGHT));
		int pixelI, pixelJ = 0;
		int PxL = eri.w;
		int PxR = 0;
		int PxU = eri.h;
		int PxD = 0;


		cams[segi] = ReferenceCamera;
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
		Vec3b insidecolor(255, 0, 0);
		cout << PxL << " " << PxR << " " << PxU << " " << PxD << endl;

		int left = newERI.cols / 2 - (((PxR - PxL)) / 2);
		int up = newERI.rows / 2 - (((PxD - PxU)) / 2);

		Rect RectangleToDraw(left, up, (PxR - PxL), (PxD - PxU));
		rectangle(newERI, RectangleToDraw.tl(), RectangleToDraw.br(), Scalar(0, 0, 255), 2, 8, 0);








		namedWindow("sample", WINDOW_NORMAL);
		resizeWindow("sample", 800, 400);

		imshow("sample",newERI);
		waitKey(10);

	}
}


void Path::RotateXYaxisERI2RERI(Mat originERI, Mat& newERI, V3 directionbefore, V3 directionaftertilt, M33 reriCS)
{
	
	ERI oERI(originERI.cols, originERI.rows);
	V3 p = directionbefore;	
	V3 p1 = directionaftertilt;
	V3 a = p.UnitVector();
	V3 b = p1.UnitVector();
	float m = p*p1;
	V3 dir = (p1^p).UnitVector();
	float angle =((float)180/(float)PI)* acos(m);
	cout << a << " " << b << " m: " << m<<" angle: " <<angle << endl;
	
	//system("pause");
	
	for (int j = 0; j < newERI.rows; j++)
	{
		for (int i = 0; i < newERI.cols; i++)
		{
			V3 q = oERI.Unproject(j,i);			
			q = reriCS * q;
			q = q.RotateThisVectorAboutDirection(dir, angle);
			int u = oERI.Lon2PixJ(oERI.GetXYZ2Longitude(q));
			int v = oERI.Lat2PixI(oERI.GetXYZ2Latitude(q));
			newERI.at<Vec3b>(j, i) = originERI.at<Vec3b>(v, u);
			
		}
	}


}