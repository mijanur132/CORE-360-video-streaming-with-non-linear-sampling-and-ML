#include"image.h"
#include<conio.h>
#include "config.h"
#include <C:\opencv\build\include\opencv2\opencv.hpp>
#include <C:\opencv\build\include\opencv2\core\core.hpp>
#include <C:\opencv\build\include\opencv2\highgui\highgui.hpp>



int Is_MInv_calculated;
M33 M_Inv;

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


void testMousecontrol()
{	
	ERI_INIT;
	int cond = 1;
	while (cond == 1)
	{

		eri.ERI2Conv(eriPixels, convPixels, camera1); 			
		imshow("CONV_image", convPixels);
		waitKey(1);

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
	
}
void testforwardbackward()
{
	ERI_INIT;
	//camera1.Tilt(120);
	eri.ERI2Conv(eriPixels, convPixels, camera1);
	imshow("CONV_image", convPixels);
	//img_write("./Image/CONV_image.png", output_image_mat);// write an image
	eri.Conv2ERI(convPixels, convPixelsreverse, eriPixels, camera1);
	imshow("ERI_image_reverse", convPixelsreverse);	
	waitKey(10000);
	
	
}

void img_write(const char *s1, cv::InputArray s2) {

	vector<int> compression_params;
	compression_params.push_back(IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(9);
	string str1 ="\""+string(s1)+ "\"";
	imwrite(s1, s2, compression_params);
	waitKey(1);
}



void playstillmanually()
{
	ERI_INIT;
	for (int i = 0; i < 1500; i++)
	{
		camera1.Pan(10);
		eri.ERI2Conv(eriPixels, convPixels, camera1);
		imshow("CONV_image", convPixels);
		waitKey(1000);

	}
}
//*/


// load  one frame and load one camera from path file and display that frame 

int out_video_file(Mat &output_image_mat, ERI eri_image, Path path1)
{

	VideoCapture cap(VIDEO);
	if (!cap.isOpened()) {
		cout << "Cannot open the video file" << endl;
		system("pause");

	}
	
	namedWindow("MyVideo", WINDOW_NORMAL);
	
	int fi = 1;
	while (1)
	{
		Mat frame;		
		cap >> frame;
		
		if (frame.empty())
		{
			cout << "empty" << endl;
			break;
		}
		path1.cams[fi].PositionAndOrient(V3(0, 0, 0), V3(0, 0, 1), V3(0, 1, 0));
		eri_image.ERI2Conv(frame, output_image_mat, path1.cams[fi]);
		imshow("MyVideo", output_image_mat);
		fi++;
		
		if (waitKey(1) >= 0)
			break;		
		char c = (char)waitKey(10);
		if (c == 27)
			break;
	}
	cout << "finish" << endl;	
	return 0;
}

//check wether interpolation works between two position of a still image. 

void check_interpolation() {
	
	PPC camera1(cFoV, cameraW, cameraH);
	PPC camera2(cFoV, cameraW, cameraH);
	camera1.Pan(90.0f);

	Mat source_image_mat;
	upload_image(IMAGE, source_image_mat);  //this function upload image of equirect form

	Mat output_image_mat = cv::Mat::zeros(cameraH, cameraW, source_image_mat.type());
	Mat output_image_mat_1 = cv::Mat::zeros(cameraH, cameraW, source_image_mat.type());
	ERI eri_image(source_image_mat.cols, source_image_mat.rows);
	eri_image.ERI2Conv(source_image_mat, output_image_mat, camera1);
	imshow("CONV_image", output_image_mat);
	waitKey(100);
	eri_image.ERI2Conv(source_image_mat, output_image_mat_1, camera2);
	imshow("CONV_image1", output_image_mat_1);
	waitKey(100);

	for (int i = 0; i < NUM_INTERP_frameN; i++)
	{
		
		//cout << i << endl;
		PPC interPPC;
		interPPC.SetInterpolated(&camera1,&camera2, i, NUM_INTERP_frameN);		
		eri_image.ERI2Conv(source_image_mat, output_image_mat, interPPC);
		imshow("CONV_imagex", output_image_mat);
		waitKey(10);
		
	}
	
}



int testPlayBackHMDPathStillImage()
{	
	Mat eriPixels; 
	char fname[] = "./Image/RollerCoasterFrame3to1.jpg";
	upload_image(fname, eriPixels);
	ERI eri(eriPixels.cols, eriPixels.rows);
	Path path1;
	PPC camera1(cFoV, cameraW, cameraH);
	Mat convPixels = cv::Mat::zeros(cameraH, cameraW, eriPixels.type());
	path1.LoadHMDTrackingData("./Video/roller.txt", camera1);
	path1.PlayBackPathStillImage(eriPixels, eri, convPixels);	
	return 0;

}


int testPlayBackManualPathStillImage() {
	ERI_INIT;
	
	int framesN = 30;
	path1.AppendCamera(camera1, framesN);
	camera1.Pan(90);
	path1.AppendCamera(camera1, framesN);
	camera1.Pan(-90);
	path1.AppendCamera(camera1, framesN);
	path1.PlayBackPathStillImage(eriPixels, eri, convPixels);	
	return 0;

}

int testPlayBackHMDPathVideo()
{
	PPC camera1(cFoV, cameraW, cameraH);	  
	Mat convPixels = cv::Mat::zeros(cameraH, cameraW, IMAGE_TYPE);	
	Path path1;
	int lastframe =1500;
	path1.LoadHMDTrackingData("./Video/roller.txt", camera1);
	path1.PlayBackPathVideo("./Video/roller.mkv", convPixels,lastframe);
	return 0;

}

int testPlayBackHMDPathVideoPixelInterval()
{
	PPC camera1(cFoV, cameraW, cameraH);
	Mat convPixels = cv::Mat::zeros(cameraH, cameraW, IMAGE_TYPE);
	Path path1;
	int lastframe = 1500;
	path1.LoadHMDTrackingData("./Video/roller.txt", camera1);
	path1.PlayBackPathVideoPixelInterval("./Video/roller_2000_1000.mp4", convPixels, lastframe);
	return 0;

}

int testViewDirectionAvgRotation() 
{
		PPC camera1(cFoV, cameraW, cameraH);
		Path path1;
		path1.LoadHMDTrackingData("./Video/diving.txt", camera1);
		path1.VDrotationAvg();
		return 0;
}


int testWriteh264() {
	Path path1;
	int lastframe = 1000;
	int codec = VideoWriter::fourcc('H', '2', '6', '4');
	path1.WriteH264("./Video/roller.mkv", lastframe, codec);
	path1.WriteH264("./Video/paris.mkv", lastframe,codec);
	path1.WriteH264("./Video/ny.mkv", lastframe, codec);
	path1.WriteH264("./Video/diving_original.mkv", lastframe, codec);

	return 0;
}



int testWriteh264tiles() {
	Path path1;
	int lastframe = 1000;	
	int codec = VideoWriter::fourcc('H', '2', '6', '4');
	path1.WriteH264tiles("./Video/roller.mkv", lastframe, 6, 4, codec);	
	path1.WriteH264tiles("./Video/paris.mkv", lastframe, 6, 4, codec);
	path1.WriteH264tiles("./Video/ny.mkv", lastframe, 6, 4, codec);
	path1.WriteH264tiles("./Video/diving_original.mkv", lastframe, 6, 4, codec);

	
	path1.WriteH264tiles("./Video/roller.mkv", lastframe, 30, 30, codec);
	path1.WriteH264tiles("./Video/paris.mkv", lastframe, 30, 30, codec);
	path1.WriteH264tiles("./Video/ny.mkv", lastframe,30, 30, codec);
	path1.WriteH264tiles("./Video/diving_original.mkv", lastframe, 30, 30, codec);
	
	
	return 0;

}

void testBoundingBox()
{
	Path path1;
	int lastFrame = 1500;
	PPC camera1(cFoV, cameraW, cameraH);
	path1.LoadHMDTrackingData("./Video/roller.txt", camera1);
	path1.DrawBoundinigBox("./video/roller.mkv", lastFrame);

}
	
void testDistortedERI()
{
	Path path1;	
	PPC camera1(cFoV, cameraW, cameraH);	
	path1.GetDistoredERI(camera1,3);

}


int testTiling() {
	PPC camera1(cFoV, cameraW, cameraH);
	PPC camera2(cFoV, cameraW, cameraH);
	PPC camera3(cFoV, cameraW, cameraH);
	PPC camera4(cFoV, cameraW, cameraH);
	PPC camera5(cFoV, cameraW, cameraH);
	Path path1;
	Path path2;
	Path path3;
	Path path4;
	Path path5;
	int lastframe = 900;

	path1.LoadHMDTrackingData("./Video/diving.txt", camera1);

	int m = 6;
	int n = 4;
	int t = 3;
	path1.ConvPixel2ERITile("./Video/diving_original.mkv", lastframe, m, n, t);

	m = 2;
	n = 1;
	t = 1;
	path1.ConvPixel2ERITile("./Video/diving_original.mkv", lastframe, m, n, t);

	m = 2;
	n = 2;
	t = 1;
	path1.ConvPixel2ERITile("./Video/diving_original.mkv", lastframe, m, n, t);

	
	m = 4;
	n = 4;
	t = 1;
	path1.ConvPixel2ERITile("./Video/diving_original.mkv", lastframe, m, n, t);

	m = 6;
	n = 6;
	t = 1;
	path1.ConvPixel2ERITile("./Video/diving_original.mkv", lastframe, m, n, t);

	m = 8;
	n = 8;
	t = 1;
	path1.ConvPixel2ERITile("./Video/diving_original.mkv", lastframe, m, n, t);

	m = 10;
	n = 10;
	t = 1;
	path1.ConvPixel2ERITile("./Video/diving_original.mkv", lastframe, m, n, t);

	m = 20;
	n = 20;
	t = 1;
	path1.ConvPixel2ERITile("./Video/diving_original.mkv", lastframe, m, n, t);


	m = 30;
	n = 30;
	t = 1;
	path1.ConvPixel2ERITile("./Video/diving_original.mkv", lastframe, m, n, t);


	m = 1;
	n = 1;
	t = 2;
	path1.ConvPixel2ERITile("./Video/diving_original.mkv", lastframe, m, n, t);

	m = 2;
	n = 1;
	t = 2;
	path1.ConvPixel2ERITile("./Video/diving_original.mkv", lastframe, m, n, t);

	m = 2;
	n = 2;
	t = 2;
	path1.ConvPixel2ERITile("./Video/diving_original.mkv", lastframe, m, n, t);


	m = 4;
	n = 4;
	t = 2;
	path1.ConvPixel2ERITile("./Video/diving_original.mkv", lastframe, m, n, t);

	m = 6;
	n = 6;
	t = 2;
	path1.ConvPixel2ERITile("./Video/diving_original.mkv", lastframe, m, n, t);

	m = 8;
	n = 8;
	t = 2;
	path1.ConvPixel2ERITile("./Video/diving_original.mkv", lastframe, m, n, t);

	m = 10;
	n = 10;
	t = 2;
	path1.ConvPixel2ERITile("./Video/diving_original.mkv", lastframe, m, n, t);

	m = 20;
	n = 20;
	t = 2;
	path1.ConvPixel2ERITile("./Video/diving_original.mkv", lastframe, m, n, t);


	m = 30;
	n = 30;
	t = 2;
	path1.ConvPixel2ERITile("./Video/diving_original.mkv", lastframe, m, n, t);

	
	m = 1;
	n = 1;
	t = 3;
	path1.ConvPixel2ERITile("./Video/diving_original.mkv", lastframe, m, n, t);

	m = 2;
	n = 1;
	t = 3;
	path1.ConvPixel2ERITile("./Video/diving_original.mkv", lastframe, m, n, t);

	m = 2;
	n = 2;
	t = 3;
	path1.ConvPixel2ERITile("./Video/diving_original.mkv", lastframe, m, n, t);


	m = 4;
	n = 4;
	t = 3;
	path1.ConvPixel2ERITile("./Video/diving_original.mkv", lastframe, m, n, t);

	m = 6;
	n = 6;
	t = 3;
	path1.ConvPixel2ERITile("./Video/diving_original.mkv", lastframe, m, n, t);

	m = 8;
	n = 8;
	t = 3;
	path1.ConvPixel2ERITile("./Video/diving_original.mkv", lastframe, m, n, t);

	m = 10;
	n = 10;
	t = 3;
	path1.ConvPixel2ERITile("./Video/diving_original.mkv", lastframe, m, n, t);

	m = 20;
	n = 20;
	t = 3;
	path1.ConvPixel2ERITile("./Video/diving_original.mkv", lastframe, m, n, t);
	

	m = 30;
	n = 30;
	t = 3;
	path1.ConvPixel2ERITile("./Video/diving_original.mkv", lastframe, m, n, t);

	//////// path 2................


	path2.LoadHMDTrackingData("./Video/italy.txt", camera2);

	m = 1;
	n = 1;
	t = 1;
	path2.ConvPixel2ERITile("./Video/italy.mkv", lastframe, m, n, t);

	m = 2;
	n = 1;
	t = 1;
	path2.ConvPixel2ERITile("./Video/italy.mkv", lastframe, m, n, t);

	m = 2;
	n = 2;
	t = 1;
	path2.ConvPixel2ERITile("./Video/italy.mkv", lastframe, m, n, t);


	m = 4;
	n = 4;
	t = 1;
	path2.ConvPixel2ERITile("./Video/italy.mkv", lastframe, m, n, t);

	m = 6;
	n = 6;
	t = 1;
	path2.ConvPixel2ERITile("./Video/italy.mkv", lastframe, m, n, t);

	m = 8;
	n = 8;
	t = 1;
	path2.ConvPixel2ERITile("./Video/italy.mkv", lastframe, m, n, t);

	m = 10;
	n = 10;
	t = 1;
	path2.ConvPixel2ERITile("./Video/italy.mkv", lastframe, m, n, t);

	m = 20;
	n = 20;
	t = 1;
	path2.ConvPixel2ERITile("./Video/italy.mkv", lastframe, m, n, t);


	m = 30;
	n = 30;
	t = 1;
	path2.ConvPixel2ERITile("./Video/italy.mkv", lastframe, m, n, t);


	m = 1;
	n = 1;
	t = 2;
	path2.ConvPixel2ERITile("./Video/italy.mkv", lastframe, m, n, t);

	m = 2;
	n = 1;
	t = 2;
	path2.ConvPixel2ERITile("./Video/italy.mkv", lastframe, m, n, t);

	m = 2;
	n = 2;
	t = 2;
	path2.ConvPixel2ERITile("./Video/italy.mkv", lastframe, m, n, t);


	m = 4;
	n = 4;
	t = 2;
	path2.ConvPixel2ERITile("./Video/italy.mkv", lastframe, m, n, t);

	m = 6;
	n = 6;
	t = 2;
	path2.ConvPixel2ERITile("./Video/italy.mkv", lastframe, m, n, t);

	m = 8;
	n = 8;
	t = 2;
	path2.ConvPixel2ERITile("./Video/italy.mkv", lastframe, m, n, t);

	m = 10;
	n = 10;
	t = 2;
	path2.ConvPixel2ERITile("./Video/italy.mkv", lastframe, m, n, t);

	m = 20;
	n = 20;
	t = 2;
	path2.ConvPixel2ERITile("./Video/italy.mkv", lastframe, m, n, t);


	m = 30;
	n = 30;
	t = 2;
	path2.ConvPixel2ERITile("./Video/italy.mkv", lastframe, m, n, t);

	//
	m = 1;
	n = 1;
	t = 3;
	path2.ConvPixel2ERITile("./Video/italy.mkv", lastframe, m, n, t);

	m = 2;
	n = 1;
	t = 3;
	path2.ConvPixel2ERITile("./Video/italy.mkv", lastframe, m, n, t);

	m = 2;
	n = 2;
	t = 3;
	path2.ConvPixel2ERITile("./Video/italy.mkv", lastframe, m, n, t);


	m = 4;
	n = 4;
	t = 3;
	path2.ConvPixel2ERITile("./Video/italy.mkv", lastframe, m, n, t);

	m = 6;
	n = 6;
	t = 3;
	path2.ConvPixel2ERITile("./Video/italy.mkv", lastframe, m, n, t);

	m = 8;
	n = 8;
	t = 3;
	path2.ConvPixel2ERITile("./Video/italy.mkv", lastframe, m, n, t);

	m = 10;
	n = 10;
	t = 3;
	path2.ConvPixel2ERITile("./Video/italy.mkv", lastframe, m, n, t);

	m = 20;
	n = 20;
	t = 3;
	path2.ConvPixel2ERITile("./Video/italy.mkv", lastframe, m, n, t);

	m = 30;
	n = 30;
	t = 3;
	path2.ConvPixel2ERITile("./Video/italy.mkv", lastframe, m, n, t);
	
	/////////////////// path 3............................

	path3.LoadHMDTrackingData("./Video/ny.txt", camera3);

	m = 1;
    n = 1;
	t = 1;
	path3.ConvPixel2ERITile("./Video/ny.mkv", lastframe, m, n, t);

	m = 2;
	n = 1;
	t = 1;
	path3.ConvPixel2ERITile("./Video/ny.mkv", lastframe, m, n, t);

	m = 2;
	n = 2;
	t = 1;
	path3.ConvPixel2ERITile("./Video/ny.mkv", lastframe, m, n, t);


	m = 4;
	n = 4;
	t = 1;
	path3.ConvPixel2ERITile("./Video/ny.mkv", lastframe, m, n, t);

	m = 6;
	n = 6;
	t = 1;
	path3.ConvPixel2ERITile("./Video/ny.mkv", lastframe, m, n, t);

	m = 8;
	n = 8;
	t = 1;
	path3.ConvPixel2ERITile("./Video/ny.mkv", lastframe, m, n, t);

	m = 10;
	n = 10;
	t = 1;
	path3.ConvPixel2ERITile("./Video/ny.mkv", lastframe, m, n, t);

	m = 20;
	n = 20;
	t = 1;
	path3.ConvPixel2ERITile("./Video/ny.mkv", lastframe, m, n, t);


	m = 30;
	n = 30;
	t = 1;
	path3.ConvPixel2ERITile("./Video/ny.mkv", lastframe, m, n, t);


	m = 1;
	n = 1;
	t = 2;
	path3.ConvPixel2ERITile("./Video/ny.mkv", lastframe, m, n, t);

	m = 2;
	n = 1;
	t = 2;
	path3.ConvPixel2ERITile("./Video/ny.mkv", lastframe, m, n, t);

	m = 2;
	n = 2;
	t = 2;
	path3.ConvPixel2ERITile("./Video/ny.mkv", lastframe, m, n, t);


	m = 4;
	n = 4;
	t = 2;
	path3.ConvPixel2ERITile("./Video/ny.mkv", lastframe, m, n, t);

	m = 6;
	n = 6;
	t = 2;
	path3.ConvPixel2ERITile("./Video/ny.mkv", lastframe, m, n, t);

	m = 8;
	n = 8;
	t = 2;
	path3.ConvPixel2ERITile("./Video/ny.mkv", lastframe, m, n, t);

	m = 10;
	n = 10;
	t = 2;
	path3.ConvPixel2ERITile("./Video/ny.mkv", lastframe, m, n, t);

	m = 20;
	n = 20;
	t = 2;
	path3.ConvPixel2ERITile("./Video/ny.mkv", lastframe, m, n, t);


	m = 30;
	n = 30;
	t = 2;
	path3.ConvPixel2ERITile("./Video/ny.mkv", lastframe, m, n, t);

	//
	m = 1;
	n = 1;
	t = 3;
	path3.ConvPixel2ERITile("./Video/ny.mkv", lastframe, m, n, t);

	m = 2;
	n = 1;
	t = 3;
	path3.ConvPixel2ERITile("./Video/ny.mkv", lastframe, m, n, t);

	m = 2;
	n = 2;
	t = 3;
	path3.ConvPixel2ERITile("./Video/ny.mkv", lastframe, m, n, t);


	m = 4;
	n = 4;
	t = 3;
	path3.ConvPixel2ERITile("./Video/ny.mkv", lastframe, m, n, t);

	m = 6;
	n = 6;
	t = 3;
	path3.ConvPixel2ERITile("./Video/ny.mkv", lastframe, m, n, t);

	m = 8;
	n = 8;
	t = 3;
	path3.ConvPixel2ERITile("./Video/ny.mkv", lastframe, m, n, t);

	m = 10;
	n = 10;
	t = 3;
	path3.ConvPixel2ERITile("./Video/ny.mkv", lastframe, m, n, t);

	m = 20;
	n = 20;
	t = 3;
	path3.ConvPixel2ERITile("./Video/ny.mkv", lastframe, m, n, t);

	m = 30;
	n = 30;
	t = 3;
	path3.ConvPixel2ERITile("./Video/ny.mkv", lastframe, m, n, t);


	////////// path 4...............
	/*

	path4.LoadHMDTrackingData("./Video/paris.txt", camera4);

	m = 1;
	n = 1;
	t = 1;
	path4.ConvPixel2ERITile("./Video/paris.mkv", lastframe, m, n, t);

	m = 2;
	n = 1;
	t = 1;
	path4.ConvPixel2ERITile("./Video/paris.mkv", lastframe, m, n, t);

	m = 2;
	n = 2;
	t = 1;
	path4.ConvPixel2ERITile("./Video/paris.mkv", lastframe, m, n, t);


	m = 4;
	n = 4;
	t = 1;
	path4.ConvPixel2ERITile("./Video/paris.mkv", lastframe, m, n, t);

	m = 6;
	n = 6;
	t = 1;
	path4.ConvPixel2ERITile("./Video/paris.mkv", lastframe, m, n, t);

	m = 8;
	n = 8;
	t = 1;
	path4.ConvPixel2ERITile("./Video/paris.mkv", lastframe, m, n, t);

	m = 10;
	n = 10;
	t = 1;
	path4.ConvPixel2ERITile("./Video/paris.mkv", lastframe, m, n, t);

	m = 20;
	n = 20;
	t = 1;
	path4.ConvPixel2ERITile("./Video/paris.mkv", lastframe, m, n, t);


	m = 30;
	n = 30;
	t = 1;
	path4.ConvPixel2ERITile("./Video/paris.mkv", lastframe, m, n, t);


	m = 1;
	n = 1;
	t = 2;
	path4.ConvPixel2ERITile("./Video/paris.mkv", lastframe, m, n, t);

	m = 2;
	n = 1;
	t = 2;
	path4.ConvPixel2ERITile("./Video/paris.mkv", lastframe, m, n, t);

	m = 2;
	n = 2;
	t = 2;
	path4.ConvPixel2ERITile("./Video/paris.mkv", lastframe, m, n, t);


	m = 4;
	n = 4;
	t = 2;
	path4.ConvPixel2ERITile("./Video/paris.mkv", lastframe, m, n, t);

	m = 6;
	n = 6;
	t = 2;
	path4.ConvPixel2ERITile("./Video/paris.mkv", lastframe, m, n, t);

	m = 8;
	n = 8;
	t = 2;
	path4.ConvPixel2ERITile("./Video/paris.mkv", lastframe, m, n, t);

	m = 10;
	n = 10;
	t = 2;
	path4.ConvPixel2ERITile("./Video/paris.mkv", lastframe, m, n, t);

	m = 20;
	n = 20;
	t = 2;
	path4.ConvPixel2ERITile("./Video/paris.mkv", lastframe, m, n, t);


	m = 30;
	n = 30;
	t = 2;
	path4.ConvPixel2ERITile("./Video/paris.mkv", lastframe, m, n, t);

	//
	m = 1;
	n = 1;
	t = 3;
	path4.ConvPixel2ERITile("./Video/paris.mkv", lastframe, m, n, t);

	m = 2;
	n = 1;
	t = 3;
	path4.ConvPixel2ERITile("./Video/paris.mkv", lastframe, m, n, t);

	m = 2;
	n = 2;
	t = 3;
	path4.ConvPixel2ERITile("./Video/paris.mkv", lastframe, m, n, t);


	m = 4;
	n = 4;
	t = 3;
	path4.ConvPixel2ERITile("./Video/paris.mkv", lastframe, m, n, t);

	m = 6;
	n = 6;
	t = 3;
	path4.ConvPixel2ERITile("./Video/paris.mkv", lastframe, m, n, t);

	m = 8;
	n = 8;
	t = 3;
	path4.ConvPixel2ERITile("./Video/paris.mkv", lastframe, m, n, t);

	m = 10;
	n = 10;
	t = 3;
	path4.ConvPixel2ERITile("./Video/paris.mkv", lastframe, m, n, t);

	m = 20;
	n = 20;
	t = 3;
	path4.ConvPixel2ERITile("./Video/paris.mkv", lastframe, m, n, t);

	m = 30;
	n = 30;
	t = 3;
	path4.ConvPixel2ERITile("./Video/paris.mkv", lastframe, m, n, t);

	//*/////////////////////// path 5..............



	path5.LoadHMDTrackingData("./Video/roller.txt", camera5);

	m = 1;
	n = 1;
	t = 1;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);

	m = 2;
	n = 1;
	t = 1;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);

	m = 2;
	n = 2;
	t = 1;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);


	m = 4;
	n = 4;
	t = 1;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);

	m = 6;
	n = 6;
	t = 1;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);

	m = 8;
	n = 8;
	t = 1;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);

	m = 10;
	n = 10;
	t = 1;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);

	m = 20;
	n = 20;
	t = 1;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);


	m = 30;
	n = 30;
	t = 1;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);


	m = 1;
	n = 1;
	t = 2;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);

	m = 2;
	n = 1;
	t = 2;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);

	m = 2;
	n = 2;
	t = 2;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);


	m = 4;
	n = 4;
	t = 2;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);

	m = 6;
	n = 6;
	t = 2;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);

	m = 8;
	n = 8;
	t = 2;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);

	m = 10;
	n = 10;
	t = 2;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);

	m = 20;
	n = 20;
	t = 2;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);


	m = 30;
	n = 30;
	t = 2;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);

	//
	m = 1;
	n = 1;
	t = 3;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);

	m = 2;
	n = 1;
	t = 3;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);

	m = 2;
	n = 2;
	t = 3;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);


	m = 4;
	n = 4;
	t = 3;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);

	m = 6;
	n = 6;
	t = 3;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);

	m = 8;
	n = 8;
	t = 3;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);

	m = 10;
	n = 10;
	t = 3;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);

	m = 20;
	n = 20;
	t = 3;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);

	m = 30;
	n = 30;
	t = 3;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);

	return 0;

}