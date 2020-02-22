#pragma once
#include"ERI.h"
#include "v3.h"
#include "ppc.h"
#include"m33.h"

#include<string.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include<vector>
#include "../../../../../OpenCV-android-sdk/sdk/native/jni/include/opencv2/core/mat.hpp"

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


void testrotationxyframe(cv::Mat &frame);
void CoRE_operation_per_frame(cv::Mat & frame, int cameraPan, int baseAngle);
