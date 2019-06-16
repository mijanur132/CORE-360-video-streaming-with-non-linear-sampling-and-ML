#pragma once
#include<vector>
#include <C:\opencv\build\include\opencv2/opencv.hpp>
#include <C:\opencv\build\include\opencv2\core\core.hpp>
#include <C:\opencv\build\include\opencv2\highgui\highgui.hpp>
#include "ERI.h"
#include <fstream>
#include<iostream>

using namespace cv;
using namespace std;

class PIXELCALCULATION {

public:
	
	int totalPixelNumber;
	int currentPixelFrame;
	vector <int>InsideLastTimeVector;
	vector<vector<int>> pixelInterval;
	PIXELCALCULATION(int totalpixelNumber);
	void NewPixelInterval(int pixel, int frame);  // pixel x of frame y start interval
	void EndPixelInterval(int pixel, int frame);  //one interval of pixel x finishes here
	void GetFramePixelInterval(ERI eri,Mat erivis, PPC *PPC);
	void PrintPixelInterval();
	void SaveIntervalTxt();

};
