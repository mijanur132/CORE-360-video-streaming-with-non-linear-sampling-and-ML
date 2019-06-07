
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


int ERI:: EachPixelConv2ERI(PPC camera1, int u, int v, int &pixelI, int &pixelJ)
{
	V3 p = camera1.GetUnitRay(0.5f + u, 0.5f + v);	//this focul length needs to go way	
	//p = p.UnitVector();
	pixelI = Lat2PixI(GetXYZ2Latitude(p));
	pixelJ = Lon2PixJ(GetXYZ2Longitude(p));

	return 0;
}





int ERI::ERI2Conv(Mat &source_image_mat, Mat &output_image_mat, PPC camera1)
{
	int pixelI, pixelJ = 0;

	for (int v = 0; v < camera1.h; v++)
	{
		for (int u = 0; u < camera1.w; u++)
		{

			EachPixelConv2ERI(camera1, u, v, pixelI, pixelJ);
			output_image_mat.at<cv::Vec3b>(v, u) = source_image_mat.at<cv::Vec3b>(pixelI, pixelJ);

		}
	}


	return 0;
}



int ERI:: Conv2ERI(Mat conv_image, Mat &output_eri_image, Mat source_eri_image, PPC camera1)
{	// two ERI image: source one is in Mat format blank one is ERI format.... this is done to use function unproject and vec3b operation. One type is suited for each one.
	for (int i = 0; i <h; ++i)
	{
		for (int j = 0; j <w; ++j)
		{
			V3 p = Unproject(i, j);
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

int ERI::ERI2Conv_forward_mapped(Mat &source_image_mat, Mat &output_image_mat, PPC camera1) {

	for (int i = 0; i < source_image_mat.rows; ++i)
	{
		for (int j = 0; j < source_image_mat.cols; ++j)
		{

			V3 p = Unproject(i, j);
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



;