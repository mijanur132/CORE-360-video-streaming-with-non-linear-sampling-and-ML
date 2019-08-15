#pragma once
#include<vector>
#include "ERI.h"
#include <fstream>
#include<iostream>
using namespace std;

#include <C:\opencv\build\include\opencv2\opencv.hpp>
#include <C:\opencv\build\include\opencv2\core\core.hpp>
#include <C:\opencv\build\include\opencv2\highgui\highgui.hpp>

//using namespace cv;


class PIXELCALCULATION {

public:
	
	int totalPixelNumber;
	int currentPixelFrame;
	vector <int>InsideLastTimeVector;
	vector<vector<int>> pixelInterval;
	PIXELCALCULATION(int totalpixelNumber);
	void NewPixelInterval(int pixel, int frame);  // pixel x of frame y start interval
	void EndPixelInterval(int pixel, int frame);  //one interval of pixel x finishes here
	void GetFramePixelInterval(ERI eri,cv::Mat erivis, PPC *PPC);
	void PrintPixelInterval();
	void SaveIntervalTxt();

};
