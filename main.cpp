#include<iostream>
#include <conio.h>
#include "ERI.h"
#include "v3.h"
#include "image.h"
#include "ppc.h"
#include <C:\opencv\build\include\opencv2/opencv.hpp>
#include <C:\opencv\build\include\opencv2\core\core.hpp>
#include <C:\opencv\build\include\opencv2\highgui\highgui.hpp>
#include <chrono> 
#include <cstdlib>


using namespace cv;
using namespace std;
using namespace std::chrono;


int main()
{	

	Mat source_image_mat;
	upload_image("./Image/360_equirectangular_800_400.jpg", source_image_mat);  //this function upload image of equirect form
	//upload_image("./Image/rectlin.jpg", source_image_mat);  //this function upload image or conv form

	ERI eri_image(source_image_mat.cols, 1, 1);
	int cameraW = 800;
	int cameraH = 400;
	PPC camera1(90.0f, cameraW, cameraH);
	Mat output_image_mat = cv::Mat::zeros(cameraH, cameraW, source_image_mat.type());
	Mat output_image_mat_reverse = cv::Mat::zeros(source_image_mat.rows, source_image_mat.cols, source_image_mat.type());
	
	camera1.Pan(0.0f);
	camera1.Tilt(0.0f);

	mouse_control(source_image_mat, output_image_mat, eri_image, camera1);  //show mouse control of ERI to CONV image conversion

	forward_backward(source_image_mat, output_image_mat, output_image_mat_reverse, eri_image, camera1); // show ERI to conv and inverse mapping
		
	img_write("./Image/source_image.png", source_image_mat);// write an image
	
	
	

	return 0;

}





//Conv2ERI(output_image_mat, output_image_mat_reverse, eri_image, camera1);   //convert regular to ERI image

//auto stop = high_resolution_clock::now();	//time stop
//cout << "Total run time:" << duration_cast<microseconds>(stop - start).count() << endl;
//imshow("output_image_rev", output_image_mat_reverse);

//imwrite("output_image_reverse", output_image_mat_reverse, compression_params);





//camera1.Tilt(45);


/*
V3 p1 = camera1.UnprojectPixel(199,199, 1);
cout << p1[0] << "," <<p1[1]<<","<<p1[2]<< endl;
// test XYZ to lat lon and latlon to pixel//

V3 p(1,0,0);
eri_image.TestXYZ2LatLong(p);
eri_image.TestLatLon2Pixel(eri_image.GetXYZ2Latitude(p), eri_image.GetXYZ2Longitude(p), source_image_mat.rows, source_image_mat.cols);

system("pause");
return 0;
//	*/


//Mat output_image_mat_reverse = cv::Mat::zeros(cameraH, cameraW, source_image_mat.type());

//auto start = high_resolution_clock::now(); //time start



//Is_MInv_calculated=0;							//Initialized for M.invert calculation 




/*
ERI2Conv_bothway_test2(source_image_mat, output_image_mat, eri_image, camera1);
system("pause");
return 0;
//*/


//ERI2Conv_back_mapped(source_image_mat,output_image_mat,eri_image,camera1);        // this function convert ERI to a regular image




