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



using namespace cv;
using namespace std;

#define NO  0
#define YES 1
#define PI  3.1416

extern int Is_MInv_calculated;
extern M33 M_Inv;

void testMousecontrol();
void testforwardbackward();
void img_write(const char *s1, cv::InputArray s2);
void playstillmanually();

int out_video_file(Mat &output_image_mat, ERI eri_image, Path path1);
void check_interpolation();

int testPlayBackHMDPathStillImage();
int testPlayBackManualPathStillImage();
int testPlayBackHMDPathVideo();