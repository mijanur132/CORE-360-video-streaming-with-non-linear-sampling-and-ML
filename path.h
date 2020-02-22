#pragma once
#include "ppc.h"
#include"ERI.h"
#include "m33.h"
#include <vector>
#include "../../../../../OpenCV-android-sdk/sdk/native/jni/include/opencv2/core/mat.hpp"

//using namespace cv;


class Path 
{

public:
	Path();
	void RotateXYaxisERI2RERI(cv::Mat & origninalERI, cv::Mat & newERI, V3 pb, V3 pa, M33 reriCS);
	vector<PPC> cams;
	vector<float> tstamps;
	vector<int> segmentFramesN;
	cv::Mat CRERI2Conv(cv::Mat & CRERI, int compressionfactor, PPC camera1, PPC refcam);
	void bilinearinterpolation(cv::Mat &distortedframemat, cv::Mat &midcorrectedmat, int row, int col, float Roriginaly, float Roriginalx);
    void CRERI2convOptimized(cv::Mat & CRERI, cv::Mat & convPixels, PPC camera1, int angle, int fi);
    void mapx();
    void nonUniformListInit();
    void updateReriCs(int baseAngle);
};
