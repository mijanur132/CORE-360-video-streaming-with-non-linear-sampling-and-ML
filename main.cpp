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

	testBugInSR(1, 1, "http://127.0.0.5:80/3vid2crf3trace/4s3s/crf30/diving", "./bwLogs/bw1962kB.txt", "./Video/source/diving.txt", 2, 3);

	
	//GenerateEncodingRegularSpecificPanTiltChunk();

	

	//GenerateEncoding4sVarSpecificPanTiltChunk();																		//dl//extraSec
	
	//testDownloadVideoHttp4thSecVar(1,1, "http://127.0.0.5:80/3vid2crf3trace/4s3s/crf40/diving", "./bwLogs/bw646kB.txt", "./Video/source/diving.txt",2,3);
	//0 for fixed direction test 1 for hmd data sets variable direction test,
										//second 0 for only default output, 1 if we want to calculate sampling rate 
										//and frame rate values: mx, min and average over all the frames of the video
	//testDownloadVideoHttp4thSecVar(1, 1, "http://127.0.0.5:80/3vid2crf3trace/1s/4s1s/roller", "./bwLogs/bw235kB.txt", "./Video/source/roller.txt", 2, 5, 1); //0 for fixed direction test 1 for hmd data sets variable direction test,
										
	//testDownloadVideoHttp(1,1);  //same conditions for the parameters as with the 4th sec var parameters
	//testDownloadVideoHttpTile(1, 1);




	STOP;


	for (int i = 0; i < 10; i++)
	{
		testCPUtime("C:/inetpub/wwwroot/3vid2crf3trace/4s3s/crf18/diving_1_10_-180.avi");
	}
	for (int i = 0; i < 10; i++)
	{
		testCPUtime("C:/inetpub/wwwroot/3vid2crf3trace/4s3s/crf30/diving_1_10_-180.avi");
	}

	for (int i = 0; i < 10; i++)
	{
		testCPUtime("C:/inetpub/wwwroot/3vid2crf3trace/4s3s/crf40/diving_1_10_-180.avi");
	}
	
	for (int i = 0; i < 10; i++)
	{
		testCPUtime("C:/inetpub/wwwroot/3vid2crf3trace/4s3s/crf50/diving_1_10_-180.avi");
	}

	
	//run10simulations();
	
	system("pause");
	return 0;
}


