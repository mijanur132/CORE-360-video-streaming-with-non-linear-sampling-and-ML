
#include"path.h"
#include"image.h"
#include<stdio.h>
#include"config.h"

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
			ERI2Conv(eriPixels, convPixels,eri,interPPC);
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

void Path::PlayBackPathVideo(ERI erivideoimage, Mat convPixels)
{	
	LoadVideoFile();

	int gfi = 0;
	int gfn = 100;

	cout << cams.size() - 1 << endl;

	for (int segi = 0; segi < cams.size() - 1; segi++)
	{
		for (int fi = 0; fi < segmentFramesN[segi] - 1; fi++)
		{
			PPC ppcL(cams[segi]);
			PPC ppcR(cams[segi + 1]);
			PPC interPPC;
			interPPC.SetInterpolated(&ppcL, &ppcR, fi, segmentFramesN[segi]);			
			ERI2Conv(allinputframe[segi], convPixels, erivideoimage, interPPC);
			string line;
			stringstream strs(line);
			strs << "frame: " << gfi << "out of: " << gfn << ends;
			imshow(line.data(), convPixels);
			waitKey(1);
			gfi++;
		}
	}

}

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
}