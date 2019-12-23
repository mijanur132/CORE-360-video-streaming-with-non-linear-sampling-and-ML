#pragma once
#include "ppc.h"
#include <vector> 
#include"ERI.h"
#include "m33.h"

//using namespace cv;


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
	int GetCamIndexUsingTime(float time_sec, int last_cam_index);
	void AppendCamera(PPC newCam, int framesN); // appends camera to cams array, connected by segment with framesN frames to the previously last camera; has to reallocate array of cams to have size increased by one, copy old data, append new data
		
	void PlayBackPathStillImage(cv::Mat ERI_image, ERI eri, cv::Mat conv_image);
	void PlayBackPathVideo(char* frame, cv::Mat convPixels, int lastFrame);
	void PlayBackPathVideoPixelInterval(char* frame, cv::Mat convPixels, int lastFrame);
	void LoadHMDTrackingData(char* fileName, PPC ppc0);
	void ConvPixel2ERITile(char *fname, int lastFrame, int m, int n, int t);
	void VDrotationAvg();
	void WriteH264(char* fname, int lastFrame, int codec);
	void WriteH264tiles(char* fname, int lastFrame, int m, int n, int codec);
	void DrawBoundinigBox(char* fname, int lastFrame);
	void DrawBoundinigBoxTemp(char* fname, int lastFrame);
	cv::Mat EncodeLinear(cv::Mat frame, struct var * var1, PPC camera1, int compressionfactor);
	cv::Mat EncodeNew(PPC camera1, int compressionfactor, cv::Mat frame, vector<float>& We, vector<float>& Het, vector<float>& Heb, vector<float>& R0x, vector<float>& R0y, vector<float>& R0R1, vector<float>& R0R4);
	cv::Mat DecodeLinear(cv::Mat encoded_image, float var[10], int compressionfactor, PPC camera1);
	void OverlayImage(cv::Mat* src, cv::Mat* overlay, const cv::Point& location);
	vector<cv::Mat> videoencode(char* fname, int lastFrame, vector<float>& We, vector<float>& Het, vector<float>& Heb, vector<float>& R0x, vector<float>& R0y, vector<float>& R0R1, vector<float>& R0R4, int compressionfactor);
	void videodecode(char* fname, int lastFrame, int original_w, int original_h, vector<float>& We, vector<float>& Het, vector<float>& Heb, vector<float>& R0x, vector<float>& R0y, vector<float>&  R0R1, vector<float>& R0R4, int compressionfactor);
	void bilinearinterpolation(cv::Mat &distortedframemat, cv::Mat &midcorrectedmat, int row, int col, float Roriginaly, float Roriginalx);
	void PixelXMapERI2RERI(int j, int uc, int w, float& u);
	void BuildERI2RERI(cv::Mat origninalERI, PPC camera1);
	void BuildERI2RERIVideo(cv::Mat origninalERI, PPC camera1);
	void RotateXYaxisERI2RERI(cv::Mat origninalERI, cv::Mat& newERI, V3 pb, V3 pa, M33 reriCS);//direction based
	void RotateXaxisERI2RERI(cv::Mat originERI, cv::Mat& newERI, PPC camera1);
	void DrawBoundinigBoxframe(cv::Mat & frame, PPC camera, float& pXl, float& pxr, float&pxu, float &pxd);
	cv::Mat EncodeNewNonLinV2(cv::Mat frame, struct var* var1, PPC camera1, PPC encodeRefPPC, int compressionfactor);
	cv::Mat EncodeNewNonLinV2Optimized(cv::Mat frame, struct var* var1, PPC camera1, PPC encodeRefPPC, int compressionfactor, int fi);
	cv::Mat DecodeNewNonLinV2(cv::Mat CRERI, float var[10], int compressionfactor, PPC camera1);
	cv::Mat CRERI2Conv(cv::Mat CRERI, float var[10], int compressionfactor, PPC camera1, PPC refcam, cv::Mat& qual, struct samplingvar * var1);
	void CRERI2ConvOptimized(cv::Mat CRERI, float var[10], ERI & eri, M33 & reriCS, cv::Mat & convPixels, int compressionfactor, PPC camera1, PPC refcam);
	void CRERI2ConvOptimizedWithSamplingRate(cv::Mat CRERI, float var[10], ERI& eri, M33& reriCS, cv::Mat& convPixels, cv::Mat& samplingPixels, int compressionfactor, PPC camera1, PPC refcam);
	void CRERI2ConvOptimizedWithSamplingRateVec(cv::Mat CRERI, float var[10], ERI & eri, M33 & reriCS, cv::Mat& samplingPixels, vector <float> & srVec,  int compressionfactor, PPC camera1, PPC refcam);
	
	void CRERI2ConvOptimizedWithSamplingRateVecMinAvg(cv::Mat CRERI, float var[10], ERI& eri, M33& reriCS, cv::Mat& samplingPixels, vector <float>& srVec, vector <float>& srVecMins, int compressionfactor, PPC camera1, PPC refcam); 
	void nonUniformListInit(float var[10]);
	void EncodingNonUniformListInit(float var[10]);
	void CRERI2ConvOptimizedWithPerFrameSamplingRateVecMinAvg(cv::Mat CRERI, float var[10], ERI& eri, M33& reriCS, cv::Mat& samplingPixels, vector <float>& srVec, vector <float>& srVecMins, int compressionfactor, PPC camera1, PPC refcam);

	void mapx(float var[10]);
	void mapxEncoding(float var[10]);
	
	void calculateAllSamplingRateOverCreri(float var[10]);
	cv::Mat CRERI2ConvQual(cv::Mat CRERI, float var[10], int compressionfactor, PPC camera1, PPC refcam, cv::Mat& qual, struct samplingvar * var1);
	
};

struct var {
	int colN;
	int rowN;
	float We;
	float Het;
};


struct samplingvar {	
	float vtin;
	float vto;
	float avg;
	float min;
};
