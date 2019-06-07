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

int ERI2Conv_forward_mapped(Mat &source_image_mat, Mat &output_image_mat, ERI eri_image, PPC camera1) {
	   
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
			}
		}
		
	}
	return 0;
}


int Conv2ERI(Mat conv_image, Mat &output_eri_image, Mat source_eri_image, ERI blank_eri_image, PPC camera1)
{	// two ERI image: source one is in Mat format blank one is ERI format.... this is done to use function unproject and vec3b operation. One type is suited for each one.
	for (int i = 0; i < blank_eri_image.h; ++i)
	{		
		for (int j = 0; j < blank_eri_image.w; ++j)
		{			
			V3 p = blank_eri_image.Unproject(i, j);
			V3 pp;
			if (!camera1.Project(p, pp))
				continue;

			if (pp[0] < conv_image.cols && pp[0] >= 0 && pp[1] >= 0 && pp[1] < conv_image.rows)
			{	
				output_eri_image.at<cv::Vec3b>(i, j) = conv_image.at<cv::Vec3b>(pp[1], pp[0]);  //pp[0]=column				
			}			

		}
	}

	return 0;

}


int EachPixelConv2ERI(ERI eri_image, PPC camera1,int u, int v, int &pixelI, int &pixelJ)
{						
			V3 p = camera1.GetUnitRay(0.5f+u,0.5f+v);	//this focul length needs to go way	
			//p = p.UnitVector();
			pixelI = eri_image.Lat2PixI(eri_image.GetXYZ2Latitude(p));
			pixelJ = eri_image.Lon2PixJ(eri_image.GetXYZ2Longitude(p));

			return 0;
}




int ERI2Conv(Mat &source_image_mat, Mat &output_image_mat, ERI eri_image, PPC camera1)
{
	int pixelI, pixelJ = 0;

	for (int v = 0; v < camera1.h; v++)
	{
		for (int u = 0; u < camera1.w; u++)
		{
			
			EachPixelConv2ERI(eri_image, camera1, u, v, pixelI, pixelJ);
			output_image_mat.at<cv::Vec3b>(v, u) = source_image_mat.at<cv::Vec3b>(pixelI, pixelJ);		
					
		}
	}
	

	return 0;
}

void mouse_control(Mat source_image_mat, Mat output_image_mat, ERI eri_image,PPC camera1)
{
	int cond = 1;
	while (cond == 1)
	{

		ERI2Conv(source_image_mat, output_image_mat, eri_image, camera1); 			
		imshow("CONV_image", output_image_mat);
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
void forward_backward(Mat source_image_mat, Mat output_image_mat, Mat output_image_mat_reverse, ERI eri_image, PPC camera1)
{
	ERI2Conv(source_image_mat, output_image_mat, eri_image, camera1);
	imshow("CONV_image", output_image_mat);
	img_write("./Image/CONV_image.png", output_image_mat);// write an image
	Conv2ERI(output_image_mat, output_image_mat_reverse, source_image_mat, eri_image, camera1);
	imshow("ERI_image_reverse", output_image_mat_reverse);
	img_write("./Image/reverse_image.png", output_image_mat_reverse);// write an image
	waitKey(0);
	system("pause");
	
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
		ERI2Conv(eriPixels, convPixels, eri, camera1);
		imshow("CONV_image", convPixels);
		waitKey(1000);

	}
}
//*/

// read the hmd data file line by line and convert each line to a new possition. Then add a camera at that position and append with the  
// path file pah1.

void read_path_file(Path &path1) {

	vector<vector<double> >     data;
	ifstream  file(HMD_DATA);
	if (!file)
	{
		cout << "error" << endl;
		system("pause");
	}
	string   line;
	while (getline(file, line))
	{
		vector<double>   lineData;
		stringstream  lineStream(line);

		double value;
		// Read an integer at a time from the line
		while (lineStream >> value)
		{
			
			// Add the integers from a line to a 1D array (vector)
			lineData.push_back(value);
		}
		// When all the integers have been read, add the 1D array
		// into a 2D array (as one line in the 2D array)
		float theta, v1, v2, v3;
		for (int i = 0; i < lineData.size(); i++)
		{

			//cout << lineData[i]<<" ";
			if (i == 2) {
				theta =  acosf(lineData[i])*180.0f/PI;
			}
			else if (i == 3) {
				v1 = lineData[i] / sin(theta);
			}
			else if (i == 4) {
				v2 = lineData[i] / sin(theta);
			}
			else if (i == 5) {
				v3 = lineData[i] / sin(theta);
			}
		}
		PPC camera1(90.0f,800, 400);
		V3 v(v2, v3, v1);
		camera1.RotateAboutAxisThroughEye(v, theta);
		path1.AppendCamera(camera1,100);
		//cout << endl;
		//cout << theta << v1 << v2 << v3<<endl;

		data.push_back(lineData);
	}


		//system("pause");
	
}

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
		ERI2Conv(frame, output_image_mat, eri_image, path1.cams[fi]);
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
	ERI eri_image(source_image_mat.cols, 1, 1);
	ERI2Conv(source_image_mat, output_image_mat, eri_image, camera1);
	imshow("CONV_image", output_image_mat);
	waitKey(100);
	ERI2Conv(source_image_mat, output_image_mat_1, eri_image, camera2);
	imshow("CONV_image1", output_image_mat_1);
	waitKey(100);

	for (int i = 0; i < NUM_INTERP_frameN; i++)
	{
		
		//cout << i << endl;
		PPC interPPC;
		interPPC.SetInterpolated(&camera1,&camera2, i, NUM_INTERP_frameN);		
		ERI2Conv(source_image_mat, output_image_mat, eri_image, interPPC);
		imshow("CONV_imagex", output_image_mat);
		waitKey(10);
		
	}
	
}


int out_video_file_interpolated(Mat &output_image_mat, ERI eri_image, Path path1)
{

	VideoCapture cap(VIDEO);
	if (!cap.isOpened()) {
		cout << "Cannot open the video file" << endl;
		system("pause");

	}

	namedWindow("MyVideo", WINDOW_NORMAL);

	vector<Mat> all_frame;

	
	int whilei = 0;
	while (whilei< NUM_FRAME_LOAD)
	{
		Mat frame;
		cap >> frame;
		whilei++;
		if (frame.empty())
		{
			cout << "empty" << endl;
			break;
		}
		all_frame.push_back(frame);

	}

	int fps = cap.get(CAP_PROP_FPS);

	
	int camera_i = FRAME_START;
	while(camera_i < all_frame.size()){

		if (NUM_INTERP_frameN == 0) {
			cout << camera_i << endl;
			ERI2Conv(all_frame[camera_i], output_image_mat, eri_image, path1.cams[camera_i]);
			imshow("MyVideo", output_image_mat);
			waitKey(1);

		
		}
		else
		{

			for (int frame_i = 0; frame_i < NUM_INTERP_frameN; frame_i++)
			{
				//cout << frame_i << endl;
				PPC interPPC;
				interPPC.SetInterpolated(&path1.cams[camera_i], &path1.cams[camera_i+ 1], frame_i, NUM_INTERP_frameN);
				ERI2Conv(all_frame[camera_i], output_image_mat, eri_image, interPPC);
				imshow("MyVideo", output_image_mat);
				waitKey(1);

			}
		}

		camera_i = camera_i + 1;
		

		if (waitKey(1000/fps) >= 0)
			break;
		char c = (char)waitKey(10);
		if (c == 27)
			break;
	}


	cout << "finish" << endl;
	return 0;
}




int testPlayBackHMDPathStillImage()
{	
	ERI_INIT;
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
	ERI_INIT;
	path1.LoadHMDTrackingData("./Video/roller.txt", camera1);
	path1.PlayBackPathVideo(erivideoimage, convPixels);	
	return 0;

}