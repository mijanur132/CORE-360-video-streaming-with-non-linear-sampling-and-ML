
#include "ERI.h"
#include "v3.h"
#include <math.h>
#include "image.h"
#include <C:\opencv\build\include\opencv2\opencv.hpp>
#include <C:\opencv\build\include\opencv2\core\core.hpp>
#include <C:\opencv\build\include\opencv2\highgui\highgui.hpp>


using namespace cv;

float xz2lonmap[2000][2000];
float xz2lon[1000000];
float xz2lat[20000];

ERI::ERI(int _w, int _h) {

	w = _w;
	h = _h;
	//cout << "One ERI created" << endl;

}


ERI::~ERI() {

	//cout << "One ERI deleteted" << endl;
	//looks at the vector class
}

// pixel centers are at (a.5, b.5)
// pixel (0, 0) has a center at (.5, .5)
float ERI::GetLongitude(int j) {

	if (j < 0 || j > w - 1)
		return FLT_MAX;

	float ret = ((float)(w-1-j) + 0.5f) / (float)w * 360.0f;

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
	p= p.UnitVector();
	
	float lat =90.0f- asin(p[1])* 180.0f / PI;
	return lat;

}

float ERI::GetXYZ2Longitude(V3 p)
{
	p= p.UnitVector();
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
	pixJ = w - 1 - pixJ;
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

int ERI::EachPixelConv2ERItemp(PPC camera1, int u, int v, int &pixelI, int &pixelJ)
{
	//
	float hfov = 110.0f;
	float corePredictionMargin = 1.2;
	int w = 1168;
	int h = 657;
	PPC camera(hfov*corePredictionMargin, w*corePredictionMargin, h*corePredictionMargin);
	camera.Pan(50.0f);
	camera.Tilt(10.0f);

	// build local coordinate system of RERI
	V3 xaxis = camera.a.UnitVector();
	V3 yaxis = camera.b.UnitVector()*-1.0f;
	V3 zaxis = xaxis ^ yaxis;
	M33 reriCS;
	reriCS[0] = xaxis;
	reriCS[1] = yaxis;
	reriCS[2] = zaxis;


	//

	V3 p = camera1.GetUnitRay(0.5f + u, 0.5f + v);	//this focul length needs to go way	
	//p = p.UnitVector();
	p = reriCS * p;
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



int ERI::ERI2Convtemp(Mat &source_image_mat, Mat &output_image_mat, PPC camera1)
{
	int pixelI, pixelJ = 0;

	for (int v = 0; v < camera1.h; v++)
	{
		for (int u = 0; u < camera1.w; u++)
		{

			EachPixelConv2ERItemp(camera1, u, v, pixelI, pixelJ);
			output_image_mat.at<cv::Vec3b>(v, u) = source_image_mat.at<cv::Vec3b>(pixelI, pixelJ);

		}
	}


	return 0;
}

int ERI::ERI2ConvDrawBorderinERI(Mat &output_image_mat, PPC _camera1, Vec3b insidecolor)
{
	PPC camera1(_camera1);
	int magf = 10;
	int linet = 35 * magf;
	camera1.a = camera1.a*(1.0f / (float)magf);
	camera1.b = camera1.b*(1.0f / (float)magf);
	camera1.w *= magf;
	camera1.h *= magf;
	int pixelI, pixelJ = 0;
	for (int v = 0; v < linet; v++)
	{
		for (int u = 0; u < camera1.w; u++)
		{
			EachPixelConv2ERI(camera1, u, v, pixelI, pixelJ);
			output_image_mat.at<cv::Vec3b>(pixelI,pixelJ) = insidecolor;
		}
	}
	
	for (int v = camera1.h - linet-1; v < camera1.h - 1; v++)
	{
		for (int u = 0; u < camera1.w; u++)
		{
			EachPixelConv2ERI(camera1, u, v, pixelI, pixelJ);
			output_image_mat.at<cv::Vec3b>(pixelI, pixelJ) = insidecolor;
		}
	}

	for (int u = 0; u < linet; u++)
	{
		for (int v = 0; v < camera1.h; v++)
		{
			EachPixelConv2ERI(camera1, u, v, pixelI, pixelJ);
			output_image_mat.at<cv::Vec3b>(pixelI, pixelJ) = insidecolor;
		}
	}
	for (int u = camera1.w - 1-linet; u < camera1.w - 1; u++)
	{
		for (int v = 0; v < camera1.h; v++)
		{
			EachPixelConv2ERI(camera1, u, v, pixelI, pixelJ);
			output_image_mat.at<cv::Vec3b>(pixelI, pixelJ) = insidecolor;
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
				output_image_mat.at<Vec3b>((int)pp[1], (int)pp[0]) = source_image_mat.at<Vec3b>(i, j);  //pp[0]=column				
			}
		}

	}
	return 0;
}

void ERI::VisualizeNeededPixels(Mat &erivis, PPC *ppc) {
	Vec3b insidecolor(0, 0, 255);
	Vec3b outsidecolor(0, 0, 0);
	int wscalefactor = w / erivis.cols;
	int hscalefactor = h / erivis.rows;
	for (int row = 0; row < erivis.rows; row++)
	{
		for (int col = 0; col < erivis.cols; col++)
		{
			int erow = row * hscalefactor;
			int ecol = col * wscalefactor;
			if (ERIPixelInsidePPC(erow, ecol, ppc))
			{
				erivis.at<Vec3b>(row, col) = insidecolor;

			}
			else {
				//erivis.at<Vec3b>(row, col) = outsidecolor;
			}
			

		}
	}
	
}

void ERI::getERIPixelsCount(Mat &erivis, PPC *ppc, uint64 &totalEriPixel) {
	Vec3b insidecolor(255, 0, 0);		
	for (int row = 0; row < erivis.rows; row++)
	{
		for (int col = 0; col < erivis.cols; col++)
		{
			int erow = row;
			int ecol = col;
			if (ERIPixelInsidePPC(row, col, ppc))
			{				
				totalEriPixel++;
			}
			
		}
	}
	//cout << "Total ERI pixel=" << totaleriPixel << endl;

}

int ERI::ERIPixelInsidePPC(int row, int col, PPC* ppc) 
{
	V3 p = Unproject(row, col);
	V3 pp;

	if (!ppc->Project(p, pp))
	{
		return 0;
	}
	if (pp[0] < 0 || pp[0] >= ppc->w || pp[1]<0 || pp[1]> ppc->h)
	{
		return 0;
	}
	return 1;

}

float ERI::GetXYZ2LatitudeOptimized(float p1)
{
    if (p1 > 1) { p1 = 1; }
    if (p1 < -1) { p1 = -1; }
    int p_1 = p1 * 10000+10000;
    float latx = xz2lat[p_1];
    return latx;
}

float ERI::GetXYZ2LongitudeOptimized(V3 p)
{
    float x = p[0];
    float z = p[2];
    int m = x * 1000 + 1000;
    int n = z * 1000 + 1000;
    float lon = xz2lonmap[m][n];
    return lon;
}

void ERI::atanvalue()
{

    for (int i = -500000; i < 500000; i++)
    {
        xz2lon[i + 500000] = atan((float)i/1000.0f);
    }


}


void ERI::xz2LatMap()
{
	for (int i = -10000; i < 10000; i++)
	{
		float ii = (float)i / 10000.0f;
		float lat = 90.0f - asin(ii)* 180.0f / PI;
		xz2lat[i + 10000] = Lat2PixI(lat);;
	}
	cout << "xz2lamap..............................." << endl;
}

void ERI::xz2LonMap()
{
	float lon;
	for (int i = -1000; i < 1000; i++)
	{
		for (int j = -1000; j < 1000; j++)
		{
			float x = (float)i / 1000.0f;
			float z = (float)j / 1000.0f;
			if (x >= 0 && z >= 0) {    // use nested if
				lon = atan(x / z)* 180.0f / PI;
			}
			else if (x < 0 && z >= 0) {
				lon = 360.0f + (atan(x / z))* 180.0f / PI;
			}
			else if (x >= 0 && z < 0) {
				lon = atan(x / z)* 180.0f / PI;
				lon = 180 + lon;
			}
			else if (x < 0 && z < 0) {
				lon = atan(x / z)* 180.0f / PI;
				lon = 180.0f + lon;			}
			else {
				cout << "this not handled yet" << endl;
				exit(0);
			}
			xz2lonmap[i + 1000][j + 1000] = Lon2PixJ(lon);
		}
	}
	cout << "xz2lonmap..............................." << endl;

}
				