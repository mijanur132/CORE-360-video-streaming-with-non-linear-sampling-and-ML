
#include"path.h"
#include"image.h"
#include<stdio.h>
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
