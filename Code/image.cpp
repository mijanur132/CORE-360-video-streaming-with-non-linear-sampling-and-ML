#pragma once
#include"image.h"
#include "v3.h"
#include "ppc.h"
#include"m33.h"
#include"ERI.h"

#include <C:\opencv\build\include\opencv2/opencv.hpp>
#include <C:\opencv\build\include\opencv2\core\core.hpp>
#include <C:\opencv\build\include\opencv2\highgui\highgui.hpp>


int Is_MInv_calculated;
M33 M_Inv;

int ERI2Conv(Mat &source_image_mat, Mat &output_image_mat, ERI eri_image, PPC camera1) {



	for (int i = 0; i < source_image_mat.rows; ++i)
	{
		for (int j = 0; j < source_image_mat.cols; ++j)
		{

			V3 p = eri_image.Unproject(i, j);
			V3 pp;
			if (!camera1.Project(p, pp))
				continue;

			if (pp[0] < camera1.w && pp[0] >= 0 && pp[1] >= 0 && pp[1] < camera1.h)
			{
				output_image_mat.at<cv::Vec3b>((int)pp[1], (int)pp[0]) = source_image_mat.at<cv::Vec3b>(i, j);  //pp[0]=column
				source_image_mat.at<cv::Vec3b>(i, j) = 0;
			}
		}
		cout << "row: " << i << "     \r";
	}

	return 0;
}


int Conv2ERI(Mat &source_image_mat, Mat &output_image_mat, ERI eri_image, PPC camera1)
{

	for (int i = 0; i < source_image_mat.rows; ++i)
	{
		for (int j = 0; j < source_image_mat.cols; ++j)
		{
					
			V3 pp;			
			V3 p= camera1.UnprojectPixel(i,j,1);
			//cout << "I,J: " << i << ","<<j<<" "<<"p:"<<p<<endl;   // there may be some issue here with i and j
			int Inew=eri_image.Lat2PixI(eri_image.GetXYZ2Latitude(p));
			int Jnew=eri_image.Lon2PixJ(eri_image.GetXYZ2Longitude(p));
			
			

			if (Jnew < camera1.w && Jnew >= 0 && Inew >= 0 && Inew < camera1.h)
			{
				output_image_mat.at<cv::Vec3b>(Inew,Jnew) = source_image_mat.at<cv::Vec3b>(i, j);
				source_image_mat.at<cv::Vec3b>(i, j) = 0;
			}
		}
		//cout << "row: " << i << "     \r";
	}

	return 0;

}




int upload_image(string path, Mat &image) {
	Mat image_temp;
	string path_temp = path;
	image_temp = imread(path_temp.c_str(), IMREAD_COLOR);
	if (!image_temp.data)                              // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	image = image_temp;


	return 0;
}