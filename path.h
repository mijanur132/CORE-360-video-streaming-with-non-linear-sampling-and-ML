#pragma once
#include "ppc.h"
#include <vector> 
#include"ERI.h"

using namespace cv;


class Path 
{

public:
	vector<PPC> cams;  // these define the views (i.e. cameras) in between which the Path interpolates		
	vector<float> tstamps;
	vector<int> segmentFramesN;  // array of same length as cams minus 1, i.e. camsN-1, defining the number of frames for each segment; segmentFramesN[0] is the number of frames from cams[0] to cams[1];
	//vector<Mat> allinputframe;
	Path();	
	~Path();
	int GetCamIndex(int fi, int fps, int segi);
	void AppendCamera(PPC newCam, int framesN); // appends camera to cams array, connected by segment with framesN frames to the previously last camera; has to reallocate array of cams to have size increased by one, copy old data, append new data
		
	void PlayBackPathStillImage(Mat ERI_image, ERI eri, Mat conv_image);
	void PlayBackPathVideo(char* frame, Mat convPixels, int lastFrame);
	void LoadHMDTrackingData(char* fileName, PPC ppc0);
	//void LoadVideoFile();
};

