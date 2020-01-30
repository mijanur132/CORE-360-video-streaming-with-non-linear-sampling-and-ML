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

void SaveAtRandomText(int count, int argv[]);
void check_interpolation();
void timeInterval(std::chrono::time_point<std::chrono::high_resolution_clock> time);
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
void testDownloadVideoHttp4thSecVar(char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int extraSec, int Size);
void testDownloadVideoHttp4thSecVarWithFatCoRE(char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int extraSec, char* fileSizeName, int FatCoRE, int accuracy);
void testDownloadVideoHttp4thSecVarAndroid(char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int extraSec, int Size);
void testSrAccrossSpecificFrame(char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int extraSec, int Size);
void testDownloadVideoHttp4thSecVarDelayFRmeasurement(char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int extraSec, int Size);
void GenerateEncoding4AllDirection();
void GenerateEncoding4sVarSpecificPanTiltChunk(int pan, int tilt, int chunkN);
void GenerateEncoding4sVarSpecificPanTiltChunk4mFileName();
void EncodeVideoWithExtensionSpecificPanTiltChunkN(char* fileName, int extraSec, float pan, float tilt, int chunkN, int extraFrame, int howManySecondsfor4thSec);
void GenerateEncodingRegularSpecificPanTiltChunk();
void GenerateEncoding4AllDirectiontemp();
void testvideoEncodeNew4stemp(int chunDurationsec, float pan, float tilt, int chunkstart);
int out_video_file();
void makeVideo4thSecVar(float pan, float tilt);
void temp_calc(vector <float> &nonUniformList, float n, int extraFrame);
void video4sBaseAndExtraSec(char* fileName4s, char* fileName1s);
void createSmallTilesFromLongTiles(char* fileName);
void testDownloadVideoHttpTile(char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int rowM, int colN);

void testDownloadVideoHttpTileBlankPixelMeasurementFoVPlus(char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int rowM, int colN);
void testDownloadVideoHttpTileDelayMeasurementFoVMobisysBoth(char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int rowM, int colN, int fovOnly, int accuracy);
void testDownloadVideoHttpTileDelayMeasurementFoVOnlyAndroid(char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int rowM, int colN);

void testDownloadVideoHttpTileDelayMeasurementFoVPlus(char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int rowM, int colN);
void testDownloadVideoHttpTileQualChngLineMeasurementFoVPlus(char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int rowM, int colN);

void testDownloadVideoHttpTileBlankPixelMeasurementFoV(char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int rowM, int colN);
void testDownloadVideoHttpTileDelayMeasurementFoV(char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int rowM, int colN);

void testBugInSR(int singleOrVariableVD, int samplingValueCalculate, char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int extraSec);
void makeVideoRegularSpecificPanTiltChunkN(string fName, float pan, float tilt, int chunkN, int beginFrame);
void run10simulations();
void testCPUtime(string srcBaseAddr);
void testCPUtimeTiles(string fileName);
void testNtileDecodeTimeSeperateVsOnAsingleFrame();
void testCombineMultipleTileIntoSingleFrame();
void testSeperateCoREinto4tiles();
void testCoREtileDecodeTimeSeperateVsOnAsingleFrame();
V3 chunkVDWithAccuracy(int frameCount, int futureFrame, Path& path1, int fps, int accuracy, int& cam_index);