#pragma once
#include "ppc.h"
#include <vector> 
#include"ERI.h"
#include "m33.h"

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
	void PlayBackPathVideoPixelInterval(char* frame, Mat convPixels, int lastFrame);
	void LoadHMDTrackingData(char* fileName, PPC ppc0);
	//void LoadVideoFile();
	void ConvPixel2ERITile(char *fname, int lastFrame, int m, int n, int t);
	void VDrotationAvg();
	void WriteH264(char* fname, int lastFrame, int codec);
	void WriteH264tiles(char* fname, int lastFrame, int m, int n, int codec);
	void DrawBoundinigBox(char* fname, int lastFrame);
	void DrawBoundinigBoxTemp(char* fname, int lastFrame);
	Mat Encode(PPC camera1, int compressionfactor, Mat frame, vector<float>& We, vector<float>& Het, vector<float>& Heb, vector<float>& R0x, vector<float>& R0y, vector<float>& R0R1, vector<float>& R0R4);
	Mat Decode(Mat encoded_image, int original_length, int original_width, float We, float Het, float Heb, float R0x, float R0y, float R0R1, float R0R4, int compressionfactor);
	void OverlayImage(Mat* src, Mat* overlay, const Point& location);
	vector<Mat> videoencode(char* fname, int lastFrame, vector<float>& We, vector<float>& Het, vector<float>& Heb, vector<float>& R0x, vector<float>& R0y, vector<float>& R0R1, vector<float>& R0R4, int compressionfactor);
	void videodecode(char* fname, int lastFrame, int original_w, int original_h, vector<float>& We, vector<float>& Het, vector<float>& Heb, vector<float>& R0x, vector<float>& R0y, vector<float>&  R0R1, vector<float>& R0R4, int compressionfactor);
	void bilinearinterpolation(Mat &distortedframemat, Mat &midcorrectedmat, int row, int col, float Roriginaly, float Roriginalx);
	void PixelXMapERI2RERI(int j, int uc, int w, float& u);
	void BuildERI2RERI(Mat origninalERI, PPC camera1);
	void BuildERI2RERIVideo(Mat origninalERI, PPC camera1);
	void RotateXYaxisERI2RERI(Mat origninalERI, Mat& newERI, V3 pb, V3 pa, M33 reriCS);//direction based
	void RotateXaxisERI2RERI(Mat originERI, Mat& newERI, PPC camera1);
};

