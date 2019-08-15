#pragma once
#include <C:\opencv\build\include\opencv2\opencv.hpp>
#include <C:\opencv\build\include\opencv2\core\core.hpp>
#include <C:\opencv\build\include\opencv2\highgui\highgui.hpp>
#include <iostream>
#include"config.h"

using namespace std;
using namespace cv;


#define C1 (float) (0.01 * 255 * 0.01  * 255)
#define C2 (float) (0.03 * 255 * 0.03  * 255)


// sigma on block_size
double sigma(Mat & m, int i, int j, int block_size);

// Covariance
double cov(Mat & m1, Mat & m2, int i, int j, int block_size);

// Mean squared error
double eqm(Mat & img1, Mat & img2);


/**
	*	Compute the PSNR between 2 images
	*/
double psnr(Mat & img_src, Mat & img_compressed, int block_size);


/**
	* Compute the SSIM between 2 images
	*/
double getssim(Mat img_src, Mat img_compressed, int block_size, bool show_progress);

void compute_quality_metrics(char * file1, char * file2, int block_size);
