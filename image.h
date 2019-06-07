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

int ERI2Conv_forward_mapped(Mat &source_image_mat, Mat &output_image_mat, ERI eri_image, PPC camera1);
int Conv2ERI(Mat conv_image, Mat &output_eri_image, Mat source_eri_image, ERI blank_eri_image, PPC camera1);
int upload_image(string path, Mat &image);  
int EachPixelConv2ERI(ERI eri_image, PPC camera1, int u, int v, int &pixelX, int &pixelY);
int ERI2Conv(Mat &source_image_mat, Mat &output_image_mat, ERI eri_image, PPC camera1);
void mouse_control(Mat source_image_mat, Mat output_image_mat, ERI eri_image, PPC camera1);
void forward_backward(Mat source_image_mat, Mat output_image_mat, Mat output_image_mat_reverse, ERI eri_image, PPC camera1);
void img_write(const char *s1, cv::InputArray s2);
void playstillmanually();
void read_path_file(Path &path1);
int out_video_file(Mat &output_image_mat, ERI eri_image, Path path1);
void check_interpolation();
int out_video_file_interpolated(Mat &output_image_mat, ERI eri_image, Path path1);
int out_video_file_interpolated_temp(Mat &output_image_mat, ERI eri_image);
int testPlayBackHMDPathStillImage();
int testPlayBackManualPathStillImage();
int testPlayBackHMDPathVideo();