
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
//using namespace cv;
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
	
		/*
		namedWindow("Display frame", WINDOW_NORMAL);
		resizeWindow("Display frame", 500, 500);
		imshow("Display frame", tileframes[0][fi]); //left top
		waitKey(10000);		

		*/		


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
		Mat erivis_noscale = Mat::zeros(eri.h, eri.w, IMAGE_TYPE);
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

		if (ret)
		{
			midPx = eri.w + (PxR + PxL) / 2;
			
			for (int u = PxL; u < PxR; u++)
			{
				for (int v = PxU; v < PxD; v++)
				{
					Vec3b insidecolor(255, 0, 0);
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
				for (int v = PxU; v < PxD; v++)
				{
					Vec3b insidecolor(255, 0, 0);
					dualframe.at<Vec3b>(v, u) = insidecolor;
					frame.at<Vec3b>(v, u) = insidecolor;
				}

			}
			for (int u = 0; u < min(PxL,PxR); u++)
			{
				for (int v = PxU; v < PxD; v++)
				{
					Vec3b insidecolor(255, 0, 0);
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
		

		eri.VisualizeNeededPixels(frame, &cams[segi]);
		namedWindow("sample", WINDOW_NORMAL);
		resizeWindow("sample", 600, 400);
		imshow("sample", frame);
		//waitKey(1000);
		//imshow("sample", dualframe);
		waitKey(10000);
		imshow("sample", midcorrectedmat);
		waitKey(10000);


	}//end of fi loop

}




