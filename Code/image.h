#pragma once
#include"ERI.h"
#include "v3.h"
#include <C:\opencv\build\include\opencv2/opencv.hpp>
#include <C:\opencv\build\include\opencv2\core\core.hpp>
#include <C:\opencv\build\include\opencv2\highgui\highgui.hpp>
#include "ppc.h"
#include"m33.h"

using namespace cv;
using namespace std;

#define NO  0
#define YES 1
#define PI  3.1416

extern int Is_MInv_calculated;
extern M33 M_Inv;

int ERI2Conv(Mat &source_image_mat, Mat &output_image_mat, ERI eri_image, PPC camera1);
int Conv2ERI(Mat &source_image_mat, Mat &output_image_mat, ERI eri_image, PPC camera1);
int upload_image(string path, Mat &image);  
int EachPixelConv2ERI(ERI eri_image, PPC camera1, int x, int y, int &pixelX, int &pixelY);
int ERI2Conv_back_mapped(Mat &source_image_mat, Mat &output_image_mat, ERI eri_image, PPC camera1);
