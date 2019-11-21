#pragma once
#include"ERI.h"
#include "v3.h"
#include "ppc.h"
#include"m33.h"
#include"path.h"
#include<string.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include<vector>

#define XTEST		1

#if XTEST
#define print(x) std::cout << x
#else
#define print(x) 
#endif 
using namespace std;
//using namespace cv;


#define NO  0
#define YES 1
#define PI  3.1416


extern int Is_MInv_calculated;
extern M33 M_Inv;

void testMousecontrol();
void testforwardbackward();
void img_write(const char *s1, cv::InputArray s2);
void playstillmanually();

int out_video_file(cv::Mat &output_image_mat, ERI eri_image, Path path1);
void check_interpolation();

int testPlayBackHMDPathStillImage();
int testPlayBackManualPathStillImage();
int testPlayBackHMDPathVideo();
int testPlayBackHMDPathVideoPixelInterval();
int testTiling();
int testViewDirectionAvgRotation();
int Writeh264VideoUptoNframes();
int testWriteh264tiles();
void testBoundingBox();
int upload_image(string path, cv::Mat &image);
void testEncodingDecoding();
void testRotationxy();
int testvideoencodedecode();
void getcheckerboard();
void testbilinear();
void testrotationxyframe();
void testvideoendecodenew();
string getChunkNametoReqnRefCam(PPC &camera, string srcBaseAddr, int chunkN, V3 cameraDirection, int tiltAngle);
void testvideoEncodeNew4s(int chunDurationsec, float pan, float tilt);
void testvideodecodeNqual();
void videowriterhelperxWithFileName(char* fileName, int chunN, float pan, float tilt, int fps, int cols, int rows, int starting_frame, int ending_frame, vector<cv::Mat> file2wr);
void videowriterhelperx(int chunN, float pan, float tilt, int fps, int cols, int rows, int starting_frame, int ending_frame, vector<cv::Mat> file2wr);
cv::Mat diffimgage(cv::Mat backgroundImage, cv::Mat currentImage);
void getssim();
void testDownloadVideoHttp(int singleOrVariableVD, int samplingValueCalculate);
//void testDownloadVideoHttp4thSecVar(int singleOrVariableVD, int samplingValueCalculate);
void testDownloadVideoHttp4thSecVar(int singleOrVariableVD, int samplingValueCalculate, char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int extraSec);
void GenerateEncoding4AllDirection();
void GenerateEncoding4sVarSpecificPanTiltChunk();
void GenerateEncodingRegularSpecificPanTiltChunk();
void GenerateEncoding4AllDirectiontemp();
void testvideoEncodeNew4stemp(int chunDurationsec, float pan, float tilt, int chunkstart);
int out_video_file();
void makeVideo4thSecVar(float pan, float tilt);
void temp_calc(vector <float> &nonUniformList, float n, int extraFrame);
void video4sBaseAndExtraSec(char* fileName4s, char* fileName1s, char* fileName2s, char* fileName3s);
void createSmallTilesFromLongTiles(char* fileName);
void testDownloadVideoHttpTile(char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int rowM, int colN);
void testDownloadVideoHttpTileMeasurement(char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int rowM, int colN);
void testBugInSR(int singleOrVariableVD, int samplingValueCalculate, char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int extraSec);
void run10simulations();
void testCPUtime(string srcBaseAddr);
void testCPUtimeTiles(string fileName);
void testNtileDecodeTimeSeperateVsOnAsingleFrame();
void testCombineMultipleTileIntoSingleFrame();
void testSeperateCoREinto4tiles();
void testCoREtileDecodeTimeSeperateVsOnAsingleFrame();