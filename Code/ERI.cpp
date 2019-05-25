
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
  h = w/2;

}

// pixel centers are at (a.5, b.5)
// pixel (0, 0) has a center at (.5, .5)
float ERI::GetLongitude(int j) {

  if (j < 0 || j > w-1)
    return FLT_MAX;

  float ret = ((float) j + 0.5f) / (float) w * 360.0f;

  return ret;
  
}

float ERI::GetLatitude(int i) {

  if (i < 0 || i > h-1)
    return FLT_MAX;

  float ret = ((float) i + 0.5f) / (float) h * 180.0f;

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
  V3 rotAxis = ((B-O) ^ y).UnitVector();
  V3 ret = B.RotateThisPointAboutArbitraryAxis(O, rotAxis, 90.0f-lt);

  return ret;

  // ERI pixel coordinates  of A are (h/2, 0)
  //                        of N are (0, 0), (0, 1), ... , (0, w-1)
  //                        of S are (h-1, 0), (h-1, 1), ... , (h-1, w-1)

}

float ERI::GetXYZ2Latitude(V3 p) 
{
	float sqrt_P = sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]);
	float x = p[0]/sqrt_P;
	float y = -p[1]/sqrt_P;
	float z = p[2]/sqrt_P;
	float lat = acos(y)* 180.0 / PI;
	return lat;

}

float ERI::GetXYZ2Longitude(V3 p)
{
	float sqrt_P = sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]);
	float x = p[0] / sqrt_P;
	float y = -p[1] / sqrt_P;
	float z = p[2] / sqrt_P;
	float lon;
	if (x >= 0 && z >= 0) {
		lon = (asin(x))* 180.0 / PI;
	}
	else if (x >= 0 && z < 0) {
		lon = (asin(x))* 180.0 / PI;
		lon = 180 - lon;
	}
	else if (x  <0 && z < 0) {
		lon = (asin(abs(x)))* 180.0 / PI;
		lon = 180+lon;
	}
	else {
		lon = (asin(abs(x)))* 180.0 / PI;
		lon = 360-lon;
	}
	return lon;
}

float ERI::TestXYZ2LatLong(V3 p)
{
	
	cout <<"lat and long:"<< GetXYZ2Latitude(p) << ","<<GetXYZ2Longitude(p) << endl;
	return 0;

}
int ERI::Lat2PixI(float lat)
{
	int pixI = lat * (float)h / 180.0f - 0.5f;

		if (pixI < 0 || pixI > h - 1)
			return FLT_MAX;
	
	return pixI;

}

int ERI::Lon2PixJ(float lon)
{
	int pixJ = lon * (float)w/360.0f - 0.5f;

	if (pixJ < 0 || pixJ > w - 1)
		return FLT_MAX;

	return pixJ;

}

int ERI::TestLatLon2Pixel(float lat, float lon)
{
	
	cout << "Pixel I and J:" << Lat2PixI(lat) << "," << Lon2PixJ(lon) << endl;
	return 0;

}


;