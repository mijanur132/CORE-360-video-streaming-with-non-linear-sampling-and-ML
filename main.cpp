#include<iostream>
#include <conio.h>
#include "ERI.h"
#include "v3.h"
#include "image.h"
#include "ppc.h"
#include "config.h"
#include <C:\opencv\build\include\opencv2\opencv.hpp>
#include <C:\opencv\build\include\opencv2\core\core.hpp>
#include <C:\opencv\build\include\opencv2\highgui\highgui.hpp>
#include <chrono> 
#include <cstdlib>
#include "path.h"


using namespace cv;
using namespace std;
using namespace std::chrono;


int main()
{/*
	Mat eriPixels; 
	//upload_image(IMAGE, eriPixels);  
	//ERI eri(eriPixels.cols, 1, 1); 
	//ERI erivideoimage(eriVw, 1, 1); 
	PPC camera1(cFoV, cameraW, cameraH); 
	Mat convPixels = cv::Mat::zeros(cameraH, cameraW, eriPixels.type()); \
	Mat convPixelsreverse = cv::Mat::zeros(cameraH, cameraW, eriPixels.type()); \
	Path path1;
	path1.LoadVideoFile(); //*/

	//testPlayBackHMDPathStillImage(); //playone still image 
	testPlayBackHMDPathVideo();   //play only one frame of the video

	return 0;
}

