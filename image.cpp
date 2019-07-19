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

void tempcheckerB()
{	
	ERI eri(1000, 1000);	
	Mat convImage;
	upload_image("./Image/cb_color.JPG", convImage);
	Mat eriMat(eri.h, eri.w, convImage.type());
	float fov[2];
	fov[0] = 90.0f;
	fov[1] = 90.0f;
	PPC camera1(fov, convImage.cols);
	camera1.PositionAndOrient(V3(0, 0, 0), V3(1, 0, 0), V3(0, 1, 0));
	
	camera1.Tilt(90);


	for (int i = 0; i < eri.h; i++)
	{
		for (int j = 0; j < eri.w; j++)
		{
			V3 p = eri.Unproject(i, j);
			V3 pp;
			if (!camera1.Project(p, pp))
				continue;
			if (pp[0] < convImage.rows && pp[0] >= 0 && pp[1] >= 0 && pp[1] < convImage.cols)
			{
				//print(pp[1] << " " << pp[0] << " " << i << " " << j<<" "<<camera1.h<<" "<<camera1.w << endl);
				eriMat.at<Vec3b>(i, j) = convImage.at<Vec3b>((int)pp[1], (int)pp[0]);
			}
		}

	}
	camera1.PositionAndOrient(V3(0, 0, 0), V3(1, 0, 0), V3(0, 1, 0));

	for (int i = 0; i < eri.h; i++)
	{
		for (int j = 0; j < eri.w; j++)
		{
			V3 p = eri.Unproject(i, j);
			V3 pp;
			if (!camera1.Project(p, pp))
				continue;
			if (pp[0] < convImage.rows && pp[0] >= 0 && pp[1] >= 0 && pp[1] < convImage.cols)
			{
				//print(pp[1] << " " << pp[0] << " " << i << " " << j<<" "<<camera1.h<<" "<<camera1.w << endl);
				eriMat.at<Vec3b>(i, j) = convImage.at<Vec3b>((int)pp[1], (int)pp[0]);
			}
		}

	}
	
	camera1.PositionAndOrient(V3(0, 0, 0), V3(1, 0, 0), V3(0, 1, 0));
	camera1.Pan(90);

	//camera1.Tilt(45);

	for (int i = 0; i < eri.h; i++)
	{
		for (int j = 0; j < eri.w; j++)
		{
			V3 p = eri.Unproject(i, j);
			V3 pp;
			if (!camera1.Project(p, pp))
				continue;
			if (pp[0] < convImage.rows && pp[0] >= 0 && pp[1] >= 0 && pp[1] < convImage.cols)
			{
				//print(pp[1] << " " << pp[0] << " " << i << " " << j<<" "<<camera1.h<<" "<<camera1.w << endl);
				eriMat.at<Vec3b>(i, j) = convImage.at<Vec3b>((int)pp[1], (int)pp[0]);
			}
		}

	}
	camera1.PositionAndOrient(V3(0, 0, 0), V3(1, 0, 0), V3(0, 1, 0));
	camera1.Pan(180);

	//camera1.Tilt(90);

	for (int i = 0; i < eri.h; i++)
	{
		for (int j = 0; j < eri.w; j++)
		{
			V3 p = eri.Unproject(i, j);
			V3 pp;
			if (!camera1.Project(p, pp))
				continue;
			if (pp[0] < convImage.rows && pp[0] >= 0 && pp[1] >= 0 && pp[1] < convImage.cols)
			{
				//print(pp[1] << " " << pp[0] << " " << i << " " << j<<" "<<camera1.h<<" "<<camera1.w << endl);
				eriMat.at<Vec3b>(i, j) = convImage.at<Vec3b>((int)pp[1], (int)pp[0]);
			}
		}

	}

	camera1.PositionAndOrient(V3(0, 0, 0), V3(1, 0, 0), V3(0, 1, 0));
	camera1.Pan(270);

	//camera1.Tilt(90);

	for (int i = 0; i < eri.h; i++)
	{
		for (int j = 0; j < eri.w; j++)
		{
			V3 p = eri.Unproject(i, j);
			V3 pp;
			if (!camera1.Project(p, pp))
				continue;
			if (pp[0] < convImage.rows && pp[0] >= 0 && pp[1] >= 0 && pp[1] < convImage.cols)
			{
				//print(pp[1] << " " << pp[0] << " " << i << " " << j<<" "<<camera1.h<<" "<<camera1.w << endl);
				eriMat.at<Vec3b>(i, j) = convImage.at<Vec3b>((int)pp[1], (int)pp[0]);
			}
		}

	}
	camera1.PositionAndOrient(V3(0, 0, 0), V3(1, 0, 0), V3(0, 1, 0));
	camera1.Tilt(-90);

	for (int i = 0; i < eri.h; i++)
	{
		for (int j = 0; j < eri.w; j++)
		{
			V3 p = eri.Unproject(i, j);
			V3 pp;
			if (!camera1.Project(p, pp))
				continue;
			if (pp[0] < convImage.rows && pp[0] >= 0 && pp[1] >= 0 && pp[1] < convImage.cols)
			{
				//print(pp[1] << " " << pp[0] << " " << i << " " << j<<" "<<camera1.h<<" "<<camera1.w << endl);
				eriMat.at<Vec3b>(i, j) = convImage.at<Vec3b>((int)pp[1], (int)pp[0]);
			}
		}

	}

	namedWindow("sample", WINDOW_NORMAL);
	resizeWindow("sample", 800, 400);
	imshow("sample", eriMat);
	waitKey(10000);

}

void testforwardbackward()
{
	Mat eriPixels; 
	upload_image(IMAGE, eriPixels);  
	ERI eri(eriPixels.cols, eriPixels.rows); 
	int cfov=90;
	PPC camera1(cfov, cameraW, cameraH); 
	Mat convPixels = Mat::zeros(cameraH, cameraW, eriPixels.type()); 
	Mat convPixelsreverse = Mat::zeros(cameraH, cameraW, eriPixels.type()); 

	//camera1.Tilt(120);
	eri.ERI2Conv(eriPixels, convPixels, camera1);


	namedWindow("sample", WINDOW_NORMAL);
	resizeWindow("sample", 800, 400);
	imshow("sample", convPixels);


	//img_write("./Image/CONV_image.png", output_image_mat);// write an image
	eri.Conv2ERI(convPixels, convPixelsreverse, eriPixels, camera1);
	namedWindow("sample1", WINDOW_NORMAL);
	resizeWindow("sample1", 800, 400);

	imshow("sample1", convPixelsreverse);	
	waitKey(10000);
	
	
}

int getcheckerboard()
{	
	Mat eriPixels;
	upload_image(IMAGE, eriPixels);
	ERI eri(eriPixels.cols, eriPixels.rows);

	float hfov = 90.0f;
	PPC camera1(hfov, hfov, 200);	
	Mat convPixels = Mat::zeros(cameraH, cameraW, eriPixels.type());
	Mat convPixelsreverse = Mat::zeros(cameraH, cameraW, eriPixels.type());

	camera1.PositionAndOrient(V3(0, 0, 0), V3(0, 0, 1), V3(0, 1, 0));

	namedWindow("sample", WINDOW_NORMAL);
	resizeWindow("sample", 800, 400);

	for (int i = 0; i <= 360; i = i + 90)
	{

		camera1.Pan(i);		
		upload_image("./Image/checkerboard2.jpg", convPixels);
		eri.Conv2ERI(convPixels, convPixelsreverse, eriPixels, camera1);

		imshow("sample1", convPixelsreverse);
		camera1.PositionAndOrient(V3(0, 0, 0), V3(0, 0, 1), V3(0, 1, 0));
		waitKey(5000);

	}

	camera1.PositionAndOrient(V3(0, 0, 0), V3(0, 0, 1), V3(0, 1, 0));
	for (int i = 0; i <= 360; i = i + 90)
	{

		camera1.Pan(i);
		camera1.Tilt(-90);
		upload_image("./Image/checkerboard2.jpg", convPixels);
		eri.Conv2ERI(convPixels, convPixelsreverse, eriPixels, camera1);

		imshow("sample1", convPixelsreverse);
		camera1.PositionAndOrient(V3(0, 0, 0), V3(0, 0, 1), V3(0, 1, 0));
		waitKey(5000);

	}

	camera1.PositionAndOrient(V3(0, 0, 0), V3(0, 0, 1), V3(0, 1, 0));
	for (int i = 0; i <= 360; i = i + 90)
	{

		camera1.Pan(i);
		camera1.Tilt(90);
		upload_image("./Image/checkerboard2.jpg", convPixels);
		eri.Conv2ERI(convPixels, convPixelsreverse, eriPixels, camera1);

		imshow("sample1", convPixelsreverse);
		camera1.PositionAndOrient(V3(0, 0, 0), V3(0, 0, 1), V3(0, 1, 0));
		waitKey(5000);

	}


	return 0;
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
	
void testbilinear()
{
	Path path1;
	Mat frame;	
	upload_image("./Image/check_rb5x5.png", frame);
	Mat outframe(10*frame.rows, 10*frame.cols, frame.type());

	for (int i = 0; i < outframe.rows; i++)
	{
		for (int j = 0; j < outframe.cols; j++)
		{
			float fi;
			float fj;
			fi = (float)(i * frame.rows)/ (float)outframe.rows;
			fj = (float)(j * frame.cols) / (float)outframe.cols;
			path1.bilinearinterpolation(outframe, frame, i, j, fi, fj);
			//outframe.at<Vec3b>(i, j) = frame.at<Vec3b>((int) fi, (int)fj);
		}

	}

	namedWindow("sample", WINDOW_NORMAL);
	resizeWindow("sample",800,800);
	imshow("sample", outframe(Rect(outframe.rows/2-50,outframe.cols / 2 - 50,100,100)));
	waitKey(10000);

}


void testEncodingDecoding()
{
	vector<float> We;
	vector<float> Het;
	vector<float> Heb;
	vector<float> R0x;
	vector<float> R0y;
	vector<float> R0R1;
	vector<float> R0R4;

	int compressionfactor = 5;
	Path path1;	
	Mat frame;
	Mat retencode;
	Mat retdecode;
	upload_image("./Image/cb_color_eri.jpg", frame);
	img_write("./Image/test_source.PNG", frame);
	float fov[2];
	fov[0] = 45.0f;
	fov[1] = 45.0f;
	PPC camera1(fov, cameraW);	
	retencode=path1.Encode(camera1,compressionfactor, frame, We, Het, Heb, R0x, R0y, R0R1, R0R4);
	namedWindow("sample", WINDOW_NORMAL);
	resizeWindow("sample", 800, 800);	
	imshow("sample", retencode);
	img_write("./Image/test_encoded.PNG", retencode);
	waitKey(1000);

	retdecode = path1.Decode(retencode, frame.cols, frame.rows, We[0], Het[0], Heb[0], R0x[0], R0y[0],R0R1[0], R0R1[4], compressionfactor);
			
	imshow("sample", retdecode);
	img_write("./Image/test_decoded.PNG", retdecode);
	waitKey(10000);
	PPC camera2(30, 1200,1200);
	//PPC camera2(30, 30, 1200);
	Mat eriPixels=retdecode;
	camera2.Tilt(90);
	ERI eri(retdecode.cols, retdecode.rows);	
	Mat convPixels = Mat::zeros(camera2.h, camera2.w, retdecode.type());	
	eri.ERI2Conv(eriPixels, convPixels, camera2);


	
	imshow("sample", convPixels);

	waitKey(10000);

}


int testvideoencodedecode() {
	
	vector<float> We;
	vector<float> Het;
	vector<float>Heb;
	vector<float> R0x;
	vector<float>R0y;
	vector<float> R0R1;
	vector<float>R0R4;
	int compressionfactor = 5;
	Path path1;
	vector <Mat> encodedbuffer;
	PPC camera1(cFoV, cameraW, cameraH);
	path1.LoadHMDTrackingData("./Video/roller.txt", camera1);
	int lastframe = 1000;	
	encodedbuffer=path1.videoencode("./Video/roller.mkv", lastframe, We, Het, Heb, R0x, R0y, R0R1, R0R4, compressionfactor);
	path1.videodecode("./Video/encodingtest/rollerh264encod.MKV", lastframe, 3840,2044, We, Het, Heb, R0x, R0y, R0R1, R0R4, compressionfactor);

	return 0;
	
}

void testRotationxy()
{
	Mat eriPixels;
	upload_image("./Image/360_equirectangular_800_400.JPG", eriPixels);
	if (!eriPixels.data)                              // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return;
	}

	
	float hfov = 110.0f;	
	PPC camera1(hfov, 800, 400);
	
	//camera1.PositionAndOrient(V3(0, 0, 0), V3(0, 0, 1), V3(0, 1, 0));
	Path path1;
	path1.BuildERI2RERI(eriPixels, camera1);

	

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