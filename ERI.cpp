
#include "ERI.h"
#include "v3.h"
#include <math.h>
#include "Image.h"
#include <C:\opencv\build\include\opencv2/opencv.hpp>
#include <C:\opencv\build\include\opencv2\core\core.hpp>
#include <C:\opencv\build\include\opencv2\highgui\highgui.hpp>


using namespace cv;

ERI::ERI(int _w, float _fps, int _framesN) {

	w = _w;
	fps = _fps;
	framesN = _framesN;
	h = w / 2;

}

// pixel centers are at (a.5, b.5)
// pixel (0, 0) has a center at (.5, .5)
float ERI::GetLongitude(int j) {

	if (j < 0 || j > w - 1)
		return FLT_MAX;

	float ret = ((float)j + 0.5f) / (float)w * 360.0f;

	return ret;

}

float ERI::GetLatitude(int i) {

	if (i < 0 || i > h - 1)
		return FLT_MAX;

	float ret = ((float)i + 0.5f) / (float)h * 180.0f;

	return ret;


}

V3 ERI::Unproject(int i, int j) {

	// convert pixel index to angles
	float lt = GetLatitude(i);
	float lg = GetLongitude(j);

	// define 3D point B on equator with given lt
	V3 A(0.0f, 0.0f, 1.0f);
	V3 O(0.0f, 0.0f, 0.0f);
	V3 y(0.0f, 1.0f, 0.0f);
	V3 B = A.RotateThisPointAboutArbitraryAxis(O, y, lg);

	// define output 3D point by rotating B up (paning) to lat;
	V3 rotAxis = ((B - O) ^ y).UnitVector();
	V3 ret = B.RotateThisPointAboutArbitraryAxis(O, rotAxis, 90.0f - lt);

	return ret;

	// ERI pixel coordinates  of A are (h/2, 0)
	//                        of N are (0, 0), (0, 1), ... , (0, w-1)
	//                        of S are (h-1, 0), (h-1, 1), ... , (h-1, w-1)

}

float ERI::GetXYZ2Latitude(V3 p)
{
	//float sqrt_P = p.Length();
	
	float lat =90.0f- asin(p[1])* 180.0f / PI;
	return lat;

}

float ERI::GetXYZ2Longitude(V3 p)
{
	//float sqrt_P = p.Length();
	float x = p[0];
	float y = p[1];
	float z = p[2];
	float lon;
	if (x >= 0 && z >= 0) {    // use nested if
		lon = (atan(x / z))* 180.0f / PI;
	}
	else if (x < 0 && z >= 0) {

		lon = 360.0f + (atan(x / z))* 180.0f / PI;

	}	
	else if (x >= 0 && z < 0) {
		lon = (atan(x/z))* 180.0f / PI;
		lon = 180+lon;
	}
	
	else if (x < 0 && z < 0) {
		lon = (atan(x/z))* 180.0f / PI;
		lon = 180.0f+lon;
	}
	
	else {
		cout << "this not handled yet" << endl;
		exit(0);
	}
	return lon;
}

float ERI::TestXYZ2LatLong(V3 p)
{

	cout << "lat and long:" << GetXYZ2Latitude(p) << "," << GetXYZ2Longitude(p) << endl;
	return 0;

}
int ERI::Lat2PixI(float lat)
{
	int pixI = (int)(lat * (float)h / 180.0f);
	/*
	if (pixI > h - 1)
	{
		pixI = h - 1;

	}
	else if (pixI < 0)
		pixI = 0;
		*/
	return pixI;

}

int ERI::Lon2PixJ(float lon)
{
	int pixJ = (int)(lon * (float)w/ 360.0f);
	/*
	if (pixJ > w - 1)
	{
		pixJ = w - 1;

	}
	else if (pixJ < 0)
		pixJ = 0;
		*/
	return pixJ;
}

int ERI::TestLatLon2Pixel(float lat, float lon, int source_H, int source_W)
{

	//cout << "Pixel I and J:" << Lat2PixI(lat, source_H) << "," << Lon2PixJ(lon, source_W) << endl;
	return 0;

}


;