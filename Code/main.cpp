#include<iostream>
#include "ERI.h"
#include "v3.h"
#include "image.h"
#include "ppc.h"
#include <C:\opencv\build\include\opencv2/opencv.hpp>
#include <C:\opencv\build\include\opencv2\core\core.hpp>
#include <C:\opencv\build\include\opencv2\highgui\highgui.hpp>
#include <chrono> 

using namespace cv;
using namespace std;
using namespace std::chrono;


int main()
{	


	PPC camera1(90.0f, 300,148);
	camera1.Pan(180.0f);
	
	

	Mat source_image_mat;
	
	//upload_image("./Image/360_equirectangular_800_400.jpg", source_image_mat);  //this function upload image of equirect form
	upload_image("./Image/rectlin.jpg", source_image_mat);  //this function upload image or conv form

	ERI eri_image(source_image_mat.cols, 1, 1);

	//// test XYZ to lat lon and latlon to pixel//	
	
	V3 p(1,0,0);	
	eri_image.TestXYZ2LatLong(p);
	eri_image.TestLatLon2Pixel(eri_image.GetXYZ2Latitude(p), eri_image.GetXYZ2Longitude(p));

	system("pause");
	return 0;
	////

	Mat output_image_mat = cv::Mat::zeros(camera1.h, camera1.w, source_image_mat.type());
	Mat output_image_mat_reverse = cv::Mat::zeros(camera1.h, camera1.w, source_image_mat.type());
	
	auto start = high_resolution_clock::now(); //time start

	Is_MInv_calculated=0;							//Initialized for M.invert calculation 
	//ERI2Conv(source_image_mat,output_image_mat,eri_image,camera1);        // this function convert ERI to a regular image
	ERI2Conv_back_mapped(source_image_mat, output_image_mat, eri_image, camera1); // convert ERI to regular using backmap
	imshow("image_source", source_image_mat);
	imshow("output_image", output_image_mat);
	//Conv2ERI(output_image_mat, output_image_mat_reverse, eri_image, camera1);   //convert regular to ERI image

	auto stop = high_resolution_clock::now();	//time stop
	cout <<"Total run time:"<< duration_cast<microseconds>(stop - start).count() << endl;

	
	imshow("output_image_reverse", output_image_mat_reverse);
	waitKey(0);
	system("pause");

	return 0;

}