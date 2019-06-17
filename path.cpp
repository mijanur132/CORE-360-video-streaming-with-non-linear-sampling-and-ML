
#include"path.h"
#include"image.h"
#include<stdio.h>
#include <stdlib.h>
#include"config.h"
#include "ERI.h"
#include "pixelCalculation.h"

using namespace std;
using namespace cv;

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
		//eri.ERI2Conv(frame, convPixels, cams[segi]);	
		//cout << fi << " " << segi << "; ";
		//imshow("outputImage", convPixels);
		eri.VisualizeNeededPixels(erivis, &(cams[segi]));
		cout << "done visualisation" << endl;
		imshow("ERIpixelsNeeded", erivis);
		waitKey(1);
	
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
	long int ERItotal = 0;
	 
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
		//eri.VisualizeNeededPixels(erivis, &(cams[segi]));
		if (fi == 1) {
			eri.getERIPixelsCount(erivis_noscale, &(cams[segi]), ERItotal);
		}
		//cout << "eritotal:"<<ERItotal << endl;
		imshow("ERIpixelsNeeded", erivis);
		waitKey(10);

		//cout << fi <<" "<< fi % (fps*t)<<" "<<fps<< endl;

		if (fi%(fps*t) == 0) 
		{
			cout <<"frame No: "<< fi << "chunk no: " << fi / (fps*t) << endl;
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
	cout << eri.h<<" "<< eri.w<<" "<<m<<" "<<n<<" "<<fps<<" "<<t<<totaltiles<< endl;
	long int total_tile_pixel = eri.h*eri.w*fps*t / (m*n);
	total_tile_pixel = totaltiles * total_tile_pixel;
	float result = (ERItotal*lastFrame * 100 /total_tile_pixel) ;
	cout << "total_tile_pixel: " << total_tile_pixel << endl;
	cout << "total requierd ERI pixel: " << ERItotal*lastFrame << endl;
	cout << "required and supplied %: " << result << endl;

}
