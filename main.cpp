#include<iostream>
#include <conio.h>
#include "ERI.h"
#include "v3.h"
#include "image.h"
#include "ppc.h"
#include "config.h"

#include <chrono> 
#include <cstdlib>
#include "path.h"

using namespace std;
using namespace std::chrono;

#include <C:\opencv\build\include\opencv2\opencv.hpp>
#include <C:\opencv\build\include\opencv2\core\core.hpp>
#include <C:\opencv\build\include\opencv2\highgui\highgui.hpp>
using namespace cv;


int main()
{
	//playstillmanually();
	//testforwardbackward();
	//testPlayBackManualPathStillImage();
	//testPlayBackHMDPathStillImage(); //playone still image 
	//testPlayBackHMDPathVideoPixelInterval();   //play only one frame of the video
	//testTiling();
	//testViewDirectionAvgRotation();
	//Writeh264VideoUptoNframes();
	
	
	
	//testBoundingBox();

	//getcheckerboard();

	//getssim();
	//testRotationxy();

	//testbilinear();
	//testEncodingDecoding();
	//testrotationxyframe();
	//out_video_file();
	//testvideoendecodenew();
	//testvideoEncodeNew4s(4);
	//testPlayBackHMDPathVideo();
	//temp_calc();
	//testWriteh264tiles();
	//GenerateEncoding4AllDirection();
	//testvideodecodeNqual();
	//testDownloadVideoHttp(1,1);  //same conditions for the parameters as with the 4th sec var parameters

	//createSmallTilesFromLongTiles("./Video/source/tileS/diving.avi_2_8.avi");

	//testSeperateCoREinto4tiles();
	//testCoREtileDecodeTimeSeperateVsOnAsingleFrame();

	testWriteh264tiles();

	//STOP;

	//testCombineMultipleTileIntoSingleFrame();
	//testNtileDecodeTimeSeperateVsOnAsingleFrame();

	//testBugInSR(1, 1, "http://127.0.0.5:80/3vid2crf3trace/4s3s/crf30/diving", "./bwLogs/bw1962kB.txt", "./Video/source/diving.txt", 2, 3);

	
	//GenerateEncodingRegularSpecificPanTiltChunk();
	//GenerateEncoding4sVarSpecificPanTiltChunk();																		//dl//extraSec
	
	
	//testDownloadVideoHttp4thSecVar(1, 1, "http://127.0.0.5:80/3vid2crf3trace/4s3s/crf30/diving", "./bwLogs/bw646kB.txt", "./Video/source/diving.txt", 1, 3);
	

										
	//testDownloadVideoHttpTile("http://127.0.0.5:80/3vid2crf3trace/tiles/diving.avi", "./bwLogs/bw1962kB.txt", "./Video/source/diving.txt",4,4, 6);
	//testDownloadVideoHttpTileMeasurement("http://127.0.0.5:80/3vid2crf3trace/tiles/crf30/diving.avi", "./bwLogs/bw1962kB.txt", "./Video/source/diving.txt", 1, 4, 6);
//	testCPUtime("C:/inetpub/wwwroot/3vid2crf3trace/4s3s/crf18/diving_1_10_-180.avi");
	//testCPUtimeTiles("C:/inetpub/wwwroot/3vid2crf3trace/Tiles/diving.avi");
	
	
	//run10simulations();
	
	system("pause");
	return 0;
}


