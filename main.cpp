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
{
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
	///testEncodingDecoding();

	//testvideoencodedecode();
	testRotationxy();
	//getcheckerboard();
	//tempcheckerB();
	//testbilinear();
	system("pause");
	return 0;
}


