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
	//testWriteh264();
	
	//testtilevideo();
	
	//testBoundingBox();

	//getcheckerboard();

	//getssim();

	//testEncodingDecoding();
	//testrotationxyframe();
	//out_video_file();
	//testvideoendecodenew();
	//testvideoEncodeNew4s(4);
	//testPlayBackHMDPathVideo();
	//temp_calc();
	//testWriteh264tiles();
	//GenerateEncoding4AllDirection();
	//GenerateEncoding4sVarSpecificPanTiltChunk();
	testDownloadVideoHttp4thSecVar(0); //0 for fixed direction test 1 for hmd data sets variable direction test

	//testRotationxy();
		
	//testbilinear();
	system("pause");
	return 0;
}


