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
	
	upload_image("./Image/360_equirectangular_4000_2000.jpg", source_image_mat);  //this function upload image of equirect form
	//upload_image("./Image/rectlin.jpg", source_image_mat);  //this function upload image or conv form

	ERI eri_image(source_image_mat.cols, 1, 1);
	int cameraW = 400;
	int cameraH = 300;
	PPC camera1(90.0f, cameraW, cameraH);
	Mat output_image_mat = cv::Mat::zeros(cameraH, cameraW, source_image_mat.type());

	
	//camera1.Pan(180.0f);

	imshow("CONV_image", output_image_mat);
	//imshow("ERI_image", source_image_mat);
	int fn = 100;

	int cond=1;	
	while (cond==1) 
	{		
		
		ERI2Conv_back_mapped_v2(source_image_mat, output_image_mat, eri_image, camera1); // convert ERI to regular using backmap	
		imshow("CONV_image", output_image_mat);
		waitKey(10);
		int ch = getch();
		cout << ch << endl;
		switch (ch)
		{
			case 77:
				camera1.Pan(5.0f);
				break;
			case 75:
				camera1.Pan(-5.0f);
				break;
			case 72:
				camera1.Tilt(5.0f);
				break;
			case 80:
				camera1.Tilt(-5.0f);
				break;
			case 113:
				cond = 0;
				break;
				
		}
	}

	
	system("pause");
	return 0;
	


	vector<int> compression_params;
	compression_params.push_back(IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(9);

	imwrite("./Image/source_image.png", source_image_mat, compression_params);
	imwrite("./Image/output_image.png", output_image_mat, compression_params);
	imshow("CONV_image", output_image_mat);
	imshow("ERI_image",source_image_mat);

	waitKey(0);
	system("pause");

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




