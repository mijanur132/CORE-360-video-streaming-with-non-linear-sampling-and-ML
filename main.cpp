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
	//GenerateEncodingRegularSpecificPanTiltChunk();

	//GenerateEncoding4sVarSpecificPanTiltChunk();
	
	//testDownloadVideoHttp4thSecVar(1,1); //0 for fixed direction test 1 for hmd data sets variable direction test,
										//second 0 for only default output, 1 if we want to calculate sampling rate 
										//and frame rate values: mx, min and average over all the frames of the video
	
										
	//testDownloadVideoHttp(1,1);  //same conditions for the parameters as with the 4th sec var parameters
	testDownloadVideoHttpTile(1, 1);
	//createSmallTilesFromLongTiles("./Video/Tiles/diving/20/24/diving_original.mkv1145656920_9_24.AVI");
	//createSmallTilesFromLongTiles("./Video/Tiles/diving/20/24/diving_original.mkv1145656920_15_24.AVI");
	//createSmallTilesFromLongTiles("./Video/Tiles/diving/20/24/diving_original.mkv1145656920_14_24.AVI");
	
	system("pause");
	return 0;
}


