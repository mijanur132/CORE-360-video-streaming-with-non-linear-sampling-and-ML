
#include"path.h"
#include"image.h"
#include<stdio.h>
#include"config.h"
#include "ERI.h"

using namespace std;
using namespace cv;

Path::Path() {
	   	 
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
	int gfi = 0;
	int gfn = 100;

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
			string line;
			stringstream strs(line);
			strs << "frame: " << gfi << "out of: " << gfn << ends;
			imshow(line.data(), convPixels);
			waitKey(1);
			gfi++;
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
	ERI eri(cap.get(CAP_PROP_FRAME_WIDTH), cap.get(CAP_PROP_FRAME_HEIGHT), 1, 1);
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
		imshow("outputImage", convPixels);
		waitKey(1);
	}	   	

	//destruct ERI here
}

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

/*
void Path::LoadVideoFile() {

	VideoCapture cap(VIDEO);
	if (!cap.isOpened()) {
		cout << "Cannot open the video file" << endl;
		system("pause");

	}
	
	int whilei = 0;
	while (whilei < NUM_FRAME_LOAD)
	{
		Mat frame;
		cap >> frame;
		whilei++;
		if (frame.empty())
		{
			cout << "empty" << endl;
			break;
		}
		allinputframe.push_back(frame);

	}

	vector<int> compression_params;
	compression_params.push_back(IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(9);	
	
	imwrite("Image.png", allinputframe[1]);
	waitKey(1);

}
//*/