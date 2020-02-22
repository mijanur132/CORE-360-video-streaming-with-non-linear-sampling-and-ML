#pragma once
#include "v3.h"
#include "ppc.h"
#include <C:\opencv\build\include\opencv2\opencv.hpp>
#include <C:\opencv\build\include\opencv2\core\core.hpp>
#include <C:\opencv\build\include\opencv2\highgui\highgui.hpp>


//using namespace cv;

class ERI 
{
public:
	ERI(int _w, int _h);
	~ERI();
	int w, h;
	float fps;
	int framesN;

	// take pixel (i, j) and convert it to (x, y, z) point on unit sphere
	// i is the row in ERI; j is the column in ERI
	V3 Unproject(int i, int j);

	// given column j, compute longitude angle (0 to 360)
	float GetLongitude(int j);
	// given row i, compute latitude angle (0 noth pole, to 180, south pole)
	float GetLatitude(int i);
	
	float GetXYZ2Latitude(V3 p);      // this will take a 3d point XYZ and convert to latiturde 
	float GetXYZ2Longitude(V3 p);  //this will take a 3d point and convert to longitude 
	float TestXYZ2LatLong(V3 p);
	int Lat2PixI(float lat);
	int Lon2PixJ(float lon);
	int TestLatLon2Pixel(float lat, float lon, int source_H, int source_W);
	int ERI2Conv(cv::Mat &source_image_mat, cv::Mat &output_image_mat, PPC camera1);
	int ERI2ConvDrawBorderinERI(cv::Mat &output_image_mat, PPC camera1, cv::Vec3b insidecolor);
	int EachPixelConv2ERI(PPC camera1, int u, int v, int &pixelI, int &pixelJ);
	int Conv2ERI(cv::Mat conv_image, cv::Mat &output_eri_image, cv::Mat source_eri_image, PPC camera1);
	int ERI2Conv_forward_mapped(cv::Mat &source_image_mat, cv::Mat &output_image_mat, PPC camera1);
	void VisualizeNeededPixels(cv::Mat &erivis, PPC *ppc);
	int ERIPixelInsidePPC(int row, int col, PPC* ppc);
	void getERIPixelsCount(cv::Mat &erivis, PPC *ppc, uint64 &totalEriPixel);
	int EachPixelConv2ERItemp(PPC camera1, int u, int v, int &pixelI, int &pixelJ);
	int ERI2Convtemp(cv::Mat &source_image_mat, cv::Mat &output_image_mat, PPC camera1);
	float GetXYZ2LatitudeOptimized(float p1);
	float GetXYZ2LongitudeOptimized(V3 p);
	void atanvalue();
	void xz2LatMap();
	void xz2LonMap();
};













	/*

		0          w-1
	0   NNNNNNNNNNNN


	h/2 AEEEEEEEEEEE


	h-1 SSSSSSSSSSSS

	Center of globe is at origin O(0.0f, 0.0f, 0.0f)
	North pole N is at (0.0f, 1.0f, 0.0f)
	South pole S is at (0.0f, -1.0f, 0.0f)
	First column of ERI, i.e. first meridien, goes through N, A(0.0f, 0.0f, 1.0f), S
	A has pixel coordinates (h/2, 0)
			y |
			  |
			  x N
			  |
			  |  x ret
			 O________________
			 /   x B             x
			xA
		   /  x S
		  /z

	*/
