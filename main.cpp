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
	//testWriteh264tiles();
	//testtilevideo();
	//testPlayBackHMDPathVideo();
	//testBoundingBox();

	//getcheckerboard();

	//getssim();

	//testEncodingDecoding();
	//testrotationxyframe();

	//testvideoendecodenew();
	//testvideoencodenew4s(4);
	testvideodecodeNcompare();

	//testRotationxy();
		
	//testbilinear();
	//system("pause");
	return 0;
}


