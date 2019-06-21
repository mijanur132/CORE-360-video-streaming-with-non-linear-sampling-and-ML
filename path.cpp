
#include"path.h"
#include"image.h"
#include<stdio.h>
#include <stdlib.h>
#include"config.h"
#include "ERI.h"
#include "pixelCalculation.h"
#include <C:\opencv\build\include\opencv2\videoio.hpp>
#include <C:\opencv\build\include\opencv2\opencv.hpp>
#include <C:\opencv\build\include\opencv2\core\core.hpp>
#include <C:\opencv\build\include\opencv2\highgui\highgui.hpp>

using namespace std;
using namespace cv;

Path::Path() {
	cout << "One Path created" << endl;
}

Path::~Path() {
	cout << "One Path deleted" << endl;
}



void Path::AppendCamera(PPC newCam, int framesN)
{
	cams.push_back(newCam);
	segmentFramesN.push_back(framesN);
}



void Path::PlayBackPathStillImage(Mat eriPixels, ERI eri, Mat convPixels) 
{
	//to play back every camera for one frame segmentFramesN[segi] needs to be 2
	//it does not reach last frame
	Mat erivis = cv::Mat::zeros(eri.h, eri.w, IMAGE_TYPE);
	cout << cams.size() - 1 << endl;	
	for (int segi = 0; segi < cams.size()-1; segi++)
	{
		for (int fi = 0; fi < segmentFramesN[segi]-1; fi++) 
		{
			PPC ppcL(cams[segi]);
			PPC ppcR(cams[segi +1]);
			PPC interPPC;
			interPPC.SetInterpolated(&ppcL, &ppcR, fi, segmentFramesN[segi]);
			eri.ERI2Conv(eriPixels, convPixels,interPPC);
			eri.VisualizeNeededPixels(erivis, &interPPC);						
			imshow("ERIpixelsNeeded", erivis);
			imshow("outimage", convPixels);
			waitKey(1);
			
		}
	}   	 
	
}

void Path::LoadHMDTrackingData(char* filename, PPC ppc0)
{
	   
	ifstream  file(filename);
	if (!file)
	{
		cout << "error: can't open file: " <<filename<< endl;
		system("pause");
	}
	string   line;
	while (getline(file, line))
	{
		stringstream  linestream(line);
		float timestamp;
		linestream >> timestamp;
		tstamps.push_back(timestamp);
		int frameindex;
		linestream >> frameindex;
		float q[4];
		linestream >> q[0] >> q[1] >> q[2] >> q[3];

		float theta2, v1, v2, v3;
		theta2 = acosf(q[0]);

		v1 = q[1] / sin(theta2);
		v2 = q[2] / sin(theta2);
		v3 = q[3] / sin(theta2);
		
		PPC camera1(ppc0);
		//camera1.PositionAndOrient(V3(0, 0, 0), V3(0, 0, 1), V3(0, 1, 0));
		V3 v(v2, v3, v1);		
		camera1.RotateAboutAxisThroughEye(v, theta2*2*180/PI);
		AppendCamera(camera1,2 );
	}

	cout << "read: " << cams.size() << " cameras from: " << filename << endl;
	
		
}



void Path::PlayBackPathVideo(char* fname, Mat convPixels, int lastFrame)
{	
	VideoCapture cap(fname);
	if (!cap.isOpened()) {
		cout << "Cannot open the video file: " <<fname<< endl;
		waitKey(100000);
		return;

	}	
	ERI eri(cap.get(CAP_PROP_FRAME_WIDTH), cap.get(CAP_PROP_FRAME_HEIGHT));
	Mat erivis = Mat::zeros(eri.h/5,eri.w/5, IMAGE_TYPE);
	int fps = cap.get(CAP_PROP_FPS);
	
	float tstep = 0;
	int segi = 0;
	for (int fi=0; fi<=lastFrame; fi++)
	{  
		Mat frame;
		cap >> frame;		
		if (frame.empty())
		{
			cout << "Can not read video frame: "<<fname<< endl;
			waitKey(100000);
			return;
		}	
		segi = GetCamIndex(fi, fps, segi);
		//eri.ERI2Conv(frame, convPixels, cams[segi]);	
		//cout << fi << " " << segi << "; ";
		//imshow("outputImage", convPixels);
		eri.VisualizeNeededPixels(erivis, &(cams[segi]));
		cout << "done visualisation" << endl;
		imshow("ERIpixelsNeeded", erivis);
		waitKey(1);
	
	}	   	
	

	//destruct ERI here
}

void Path::PlayBackPathVideoPixelInterval(char* fname, Mat convPixels, int lastFrame)
{

	VideoCapture cap(fname);
	if (!cap.isOpened()) {
		cout << "Cannot open the video file: " << fname << endl;
		waitKey(100000);
		return;

	}
	
	ERI eri(cap.get(CAP_PROP_FRAME_WIDTH), cap.get(CAP_PROP_FRAME_HEIGHT));
	Mat erivis = Mat::zeros(eri.h / 5, eri.w / 5, IMAGE_TYPE);
	int fps = cap.get(CAP_PROP_FPS);
	int totalpixelnumber = eri.h*eri.w;
	PIXELCALCULATION pixelcalc(totalpixelnumber);

	float tstep = 0;
	int segi = 0;
	for (int fi = 0; fi <= lastFrame; fi++)
	{
		Mat frame;
		cap >> frame;
		if (frame.empty())
		{
			cout << "Can not read video frame: " << fname << endl;
			waitKey(100000);
			return;
		}
		segi = GetCamIndex(fi, fps, segi);	
		pixelcalc.GetFramePixelInterval(eri, erivis, &(cams[segi]));		
		cout << "done visualisation" << endl;		
		waitKey(1);

	}

	//pixelcalc.PrintPixelInterval();
	pixelcalc.SaveIntervalTxt();
	cout << "done Interval saving" << endl;
}//most outer loop of playbackpathvideoopixelinterval




int Path::GetCamIndex(int fi, int fps, int segi) {

	int ret = segi;
	while (tstamps[ret] <= (float)(fi)/(float)(fps))
	{
		ret++;
		if (ret >= tstamps.size())
		{
			cerr<<"Reached the end of the path without finding HMD pos for frame"<<endl;
			waitKey(10000);
			exit(0);

		}

	}

	return ret - 1;

}

void Path::ConvPixel2ERITile(char *fname, int lastFrame, int m, int n, int t)
{
	VideoCapture cap(fname);
	if (!cap.isOpened())
	{
		cout << "Can not open the video file: " << fname << endl;
		waitKey(100000);
		return;
	}
	int scale = 5;
	ERI eri(cap.get(CAP_PROP_FRAME_WIDTH), cap.get(CAP_PROP_FRAME_HEIGHT));
	Mat erivis = Mat::zeros(eri.h / scale, eri.w / scale, IMAGE_TYPE);
	Mat erivis_noscale = Mat::zeros(eri.h, eri.w, IMAGE_TYPE);
	vector <int> Tilebitmap;
	
	for (int i = 0; i < m*n; i++)
	{
		Tilebitmap.push_back(0);
	}
	int fps = cap.get(CAP_PROP_FPS);

	int segi = 0;
	int totaltiles = 0;
	uint64 ERItotal = 0;
	 
	for (int fi = 1; fi < lastFrame+1; fi++)
	{
		Mat frame;
		cap >> frame;
		
		if (frame.empty())
		{
			cout << "Can not read the frame" << endl;
			waitKey(100000);
			return;
		}
		segi = GetCamIndex(fi, fps, segi);
		int pixelI, pixelJ = 0;
		for (int v = 0; v < cams[segi].h; v++) 
		{
			for (int u = 0; u < cams[segi].w; u++) 
			{
				eri.EachPixelConv2ERI(cams[segi], u, v, pixelI, pixelJ);
				int Xtile = floor(pixelJ*m / eri.w); //m*n row and column
				int Ytile = floor(pixelI*n / eri.h);
				int vectorindex = (Ytile)*m + Xtile;
				Tilebitmap.at(vectorindex) = 1;

			}				
		
		}
		
		for (int row = 0; row < erivis.rows; row++)
		{
			for (int col = 0; col < erivis.cols; col++)
			{
				int Xtile = floor(col*m*scale / eri.w); //m*n row and column
				int Ytile = floor(row*n*scale / eri.h);				
				Vec3b insidecolor(25 * (Xtile + 1)*(Ytile + 1), 0, 0);
				erivis.at<Vec3b>(row, col) = insidecolor;

				
			}
		}
		//eri.VisualizeNeededPixels(erivis, &(cams[segi]));
		if (fi == 1) {
			eri.getERIPixelsCount(erivis_noscale, &(cams[segi]), ERItotal);
		}
		//cout << "eritotal:"<<ERItotal << endl;
		//imshow("ERIpixelsNeeded", erivis);
		//waitKey(10);

		//cout << fi <<" "<< fi % (fps*t)<<" "<<fps<< endl;

		if (fi%(fps*t) == 0) 
		{
			//cout <<"frame No: "<< fi << "chunk no: " << fi / (fps*t) << endl;
			for (int i = 0; i < m*n; i++)
			{
				
				if (Tilebitmap[i]==1)
				{
					totaltiles++;
				}
				//cout << "tile: " << Tilebitmap[i] <<", Total-"<<totaltiles<< endl;
				Tilebitmap.at(i)=0;
			}
			waitKey(10);
		}

		
	}//outer for loop for all frame
	cout <<"H: "<< eri.h<<" W: "<< eri.w<<" M: "<<m<<" N: "<<n<<" FPS: "<<fps<<" "<<" T: "<<t<<" Total Tiles: "<<totaltiles<< endl;
	double total_tile_pixel = eri.h*eri.w*fps*t*1UL / (m*n);
	total_tile_pixel = totaltiles * total_tile_pixel*1UL;
	double result = (ERItotal*lastFrame * 100UL /total_tile_pixel) ;
	//cout << "Total_tile_pixel: " << total_tile_pixel << endl;
	//cout << "Total requierd ERI pixel: " << ERItotal*lastFrame << endl;
	cout << "Required and supplied %: " << result << endl;

}

void Path::VDrotationAvg()
{	
	double total_angle=0;
	float angle=0;
	for (int i = 0; i < cams.size(); i++)
	{
		V3 cam1 = cams[i].GetVD();
		V3 cam2 = cams[i + 1].GetVD();
		cout << cam1 <<"; " <<cam2<< endl;
		double angleproduct = cam1 * cam2;
		if (angleproduct <= -1.0) {
			angle=180;
		}
		else if (angleproduct >= 1.0) {
			angle= 0;
		}
		else {
			angle = acos(cam1*cam2)* 180.0f / PI;
		}
		
		total_angle = total_angle + angle;
		cout << "Angle: " << angle << " Total angle: " << total_angle << endl;
	
	}
	
	cout << "No of camera: " << cams.size() << "; Total Angle: " << total_angle << endl;
}

void Path::WriteH264(char* fname, int lastFrame)
{

	VideoCapture cap(fname);
	if (!cap.isOpened()) 
	{
		cout << "Cannot open the video file: " << fname << endl;
		waitKey(100000);
		return;

	}	

	int frame_width = cap.get(CAP_PROP_FRAME_WIDTH);
	int frame_height = cap.get(CAP_PROP_FRAME_HEIGHT);
	int fps = cap.get(CAP_PROP_FPS);
	string filename = "./Video/rollerh264.avi";
	VideoWriter writer;

	int codec = VideoWriter::fourcc('X','2','6','4');
	writer.set(VIDEOWRITER_PROP_QUALITY, 2000);
	writer.open(filename, codec, fps,Size(frame_width,frame_height), true);
	
	cout<<writer.get(VIDEOWRITER_PROP_QUALITY)<<endl;

	if (!writer.isOpened())
	{
		cerr << "Could not open the output video file for write\n";
		return;
	}

	cout << "Writing videofile: " << filename << endl;

	for (int fi = 0; fi <= lastFrame; fi++)
	{
		Mat frame;
		cap >> frame;
		if (frame.empty())
		{
			cout << fi << endl;
			cout << "Can not read video frame: " << fname << endl;
			waitKey(100000);
			return;
		}
		
		writer.write(frame);		
	}
}


void Path::WriteH264tiles(char* fname, int lastFrame, int m, int n)
{

	VideoCapture cap(fname);
	if (!cap.isOpened())
	{
		cout << "Cannot open the video file: " << fname << endl;
		waitKey(100000);
		return;

	}

	int frame_width = cap.get(CAP_PROP_FRAME_WIDTH);
	int frame_height = cap.get(CAP_PROP_FRAME_HEIGHT);
	int fps = cap.get(CAP_PROP_FPS);	
	int codec = VideoWriter::fourcc('H', '2', '6', '4');

	vector<vector <Mat>> tileframes;
	for (int i = 0; i < m*n; i++)
	{		
		tileframes.push_back(vector<Mat>());

	}


	for (int fi = 0; fi <= lastFrame; fi++)
	{
		Mat frame;
		cap >> frame;
		if (frame.empty())
		{
			cout << fi << endl;
			cout << "Can not read video frame: " << fname << endl;
			waitKey(100000);
			return;
		}

		int Npx = frame_width / m;
		int Npy = frame_height / n;
		int tilesi = 0;
		

		for (int i = 0; i < frame.rows; i += Npy)
		{		
			for (int j = 0; j < frame.cols; j += Npx)
			{
				Mat tile = frame(Range(i, min(i + Npy, frame.rows)),
					Range(j, min(j + Npx, frame.cols)));
				tileframes[tilesi].push_back(tile);
				tilesi++;
			}
			
		}		
		/*
		namedWindow("Display frame", WINDOW_NORMAL);
		resizeWindow("Display frame", 500, 500);
		imshow("Display frame", tileframes[0][fi]); //left top
		waitKey(10000);
		imshow("Display frame", tileframes[1][fi]); //right top
		waitKey(10000);
		imshow("Display frame", tileframes[2][fi]); //left bottom
		waitKey(10000);
		imshow("Display frame", tileframes[3][fi]);
		waitKey(10000);

		*/		
		
	}


	for (int i = 0; i < 1; i++)
	{
		cout << "Writing videotile: " << i << endl;
		std::ostringstream oss;
		oss << "./Video/rollerh264output" << i << ".avi";
		//string filename = oss.str();
		string filename = "./Video/rollerh264output.avi";
		VideoWriter writer;

		writer.open(filename, codec, fps, Size(tileframes[1][1].rows, tileframes[1][1].cols), true);
		if (!writer.isOpened())
		{
			cerr << "Could not open the output video file for write\n";
			return;
		}
		for (int j = 0; j < lastFrame; j++)
		{				
			writer.write(tileframes[i][j]);
			namedWindow("Display frame", WINDOW_NORMAL);
			resizeWindow("Display frame", 500, 500);
			imshow("Display frame", tileframes[i][j]); //left top
			waitKey(100);
		}

		writer.release();

	}

	return;
}




void Path::Playtilevideo(char* fname, int lastFrame)
{
	VideoCapture cap(fname);
	if (!cap.isOpened()) {
		cout << "Cannot open the video file: " << fname << endl;
		waitKey(100000);
		return;

	}
	

	for (int fi = 0; fi <= lastFrame; fi++)
	{
		Mat frame;
		cap >> frame;
		if (frame.empty())
		{
			cout << "Can not read video frame: " << fname << endl;
			waitKey(100000);
			return;
		}
		
		imshow("outputImage", frame);		
		waitKey(10);

	}
	
}


void Path::WriteH264tilestemp(char* fname, int lastFrame, int m, int n)
{

	VideoCapture cap(fname);
	if (!cap.isOpened())
	{
		cout << "Cannot open the video file: " << fname << endl;
		waitKey(100000);
		return;

	}

	int frame_width = cap.get(CAP_PROP_FRAME_WIDTH);
	int frame_height = cap.get(CAP_PROP_FRAME_HEIGHT);
	int fps = cap.get(CAP_PROP_FPS);
	int codec = VideoWriter::fourcc('H', '2', '6', '4');

	int Npx = frame_width / m;
	int Npy = frame_height / n;

	string filename = "./Video/rollerh264output0.avi";
	VideoWriter writer1;
	writer1.open(filename, codec, fps, Size(Npx, Npy), true);
	Mat frame;
	Mat tile;
	cout << "first tile" << endl;

	for (int fi = 0; fi <= lastFrame; fi++)
	{
		
		cap >> frame;
		if (frame.empty())
		{
			cout << fi << endl;
			cout << "Can not read video frame: " << fname << endl;
			waitKey(100000);
			return;
		}

		if (!writer1.isOpened())
		{
			cerr << "Could not open the output video file for write\n";
			return;
		}

	
		tile = frame(Range(0,Npy),Range(0,Npx));
		writer1.write(tile);
		namedWindow("Display frame", WINDOW_NORMAL);
		resizeWindow("Display frame", 500, 500);
		//imshow("Display frame", tile); //left top
		//waitKey(10);			

	} // end fi loop for all frame// end fi loop for all frame

	writer1.release();
	////////////////////////////////////////////////////////////////////////////////////////
	VideoCapture cap1(fname);
	if (!cap1.isOpened())
	{
		cout << "Cannot open the video file: " << fname << endl;
		waitKey(100000);
		return;

	}

	filename = "./Video/rollerh264output1.avi";
	VideoWriter writer2;
	writer2.open(filename, codec, fps, Size(Npx, Npy), true);

	cout << "second tile" << endl;

	for (int fi = 0; fi <= lastFrame; fi++)
	{
		
		cap1 >> frame;
		if (frame.empty())
		{
			cout << fi << endl;
			cout << "Can not read video frame: " << fname << endl;
			waitKey(100000);
			return;
		}

		if (!writer2.isOpened())
		{
			cerr << "Could not open the output video file for write\n";
			return;
		}
		


		tile = frame(Range(Npy, 2*Npy), Range(0, Npx));
		writer2.write(tile);
		namedWindow("Display frame", WINDOW_NORMAL);
		resizeWindow("Display frame", 500, 500);
		//imshow("Display frame", tile); //left top
		//waitKey(10);

	} // end fi loop for all frame// end fi loop for all frame

	writer2.release();
//////////////////////////////////////////////////////////////////////////////////
	cout << "3rdtile" << endl;
	VideoCapture cap2(fname);
	if (!cap2.isOpened())
	{
		cout << "Cannot open the video file: " << fname << endl;
		waitKey(100000);
		return;

	}

	filename = "./Video/rollerh264output2.avi";
	VideoWriter writer3;
	writer3.open(filename, codec, fps, Size(Npx, Npy), true);



	for (int fi = 0; fi <= lastFrame; fi++)
	{
		frame;
		cap2 >> frame;
		if (frame.empty())
		{
			cout << fi << endl;
			cout << "Can not read video frame: " << fname << endl;
			waitKey(100000);
			return;
		}

		if (!writer3.isOpened())
		{
			cerr << "Could not open the output video file for write\n";
			return;
		}



		tile = frame(Range(Npy, 2*Npy), Range(Npx, 2*Npx));
		writer3.write(tile);
		namedWindow("Display frame", WINDOW_NORMAL);
		resizeWindow("Display frame", 500, 500);
		//imshow("Display frame", tile); //left top
		//waitKey(10);

	} // end fi loop for all frame// end fi loop for all frame

	writer3.release();
///////////////////////////////////////////////////////////////////

	VideoCapture cap3(fname);
	if (!cap3.isOpened())
	{
		cout << "Cannot open the video file: " << fname << endl;
		waitKey(100000);
		return;

	}

	filename = "./Video/rollerh264output3.avi";
	VideoWriter writer4;
	writer4.open(filename, codec, fps, Size(Npx, Npy), true);



	for (int fi = 0; fi <= lastFrame; fi++)
	{
		frame;
		cap3 >> frame;
		if (frame.empty())
		{
			cout << fi << endl;
			cout << "Can not read video frame: " << fname << endl;
			waitKey(100000);
			return;
		}

		if (!writer4.isOpened())
		{
			cerr << "Could not open the output video file for write\n";
			return;
		}


		tile = frame(Range(0,Npy), Range(Npx, 2*Npx));
		writer4.write(tile);
		namedWindow("Display frame", WINDOW_NORMAL);
		resizeWindow("Display frame", 500, 500);
		//imshow("Display frame", tile); //left top
		//waitKey(10);

	} // end fi loop for all frame// end fi loop for all frame

	writer4.release();



	   	 
	return;
}