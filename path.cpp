#include <iostream>
#include"path.h"
#include"image.h"
#include<stdio.h>
#include<math.h>
#include <stdlib.h>
#include <android/log.h>
#include "ERI.h"
#include "../../../../../OpenCV-android-sdk/sdk/native/jni/include/opencv2/core/mat.hpp"


#include <C:\opencv\build\include\opencv2\videoio.hpp>
#include <C:\opencv\build\include\opencv2\opencv.hpp>
#include <C:\opencv\build\include\opencv2\core\core.hpp>
#include <C:\opencv\build\include\opencv2\highgui\highgui.hpp>

using namespace cv;

vector <float> nonuniformDrev;
vector <float> nonuniformDrev2;
float colMap[3850][3850];//ch
float rowMap[3850][3850];//ch
ERI eri(3840, 2048);
M33 reriCS;
int predictMargin=1;
float wee=268.9;
float hett=122.3;

Path::Path() {
	print("One Path created" << endl);
}

void Path::updateReriCs(int baseAngle){
	int frameLen = 3840;
	int frameWidth = 2048;
	float hfov = 90.0f;
	float corePredictionMargin =  predictMargin;
	int compressionFactor = 5;
	int w = frameLen * hfov / 360;
	int h = frameWidth * hfov / 360;
	PPC camera(hfov*corePredictionMargin, w*corePredictionMargin, h*corePredictionMargin);
	camera.Pan(baseAngle);
	V3 xaxis = camera.a.UnitVector();
	V3 yaxis = camera.b.UnitVector()*-1.0f;
	V3 zaxis = xaxis ^ yaxis;

	reriCS[0] = xaxis;
	reriCS[1] = yaxis;
	reriCS[2] = zaxis;


}
void Path::nonUniformListInit()
{
    cout << "................. "<< endl;
    int compressionfactor = 5;
    float R0x = wee * 5;
    float R0y =hett* 5;
    //cout << var[2] << " " << var[3] << endl;
    for (int col = 0; col <R0x; col++)
    {
        float j = (float)col / (float)R0x;
        float a = 1 - (float)1 / (float)compressionfactor - 0;
        float b = (1 - a);
        a = a * (-1);
        float quad_out = R0x * (b - (float)sqrt(b*b - 4 * a*j)) / (float)(2 * a*compressionfactor);
        //cout << a << " " << b << " " << quad_out << endl;
        nonuniformDrev.push_back(quad_out);
    }

    for (int row = 0; row < R0y; row++)
    {
        float j = (float)row / (float)R0y;
        float a = 1 - ((float)1 / (float)compressionfactor);
        float b = (1 - a);
        a = a * (-1);
        float quad_out = R0y * (b - (float)sqrt(b*b - 4 * a*j)) / (float)(2 * a*compressionfactor);
        //cout << a << " " << b << " " << quad_out << endl;
        nonuniformDrev2.push_back(quad_out);
    }
}

void Path::mapx()
{
	int compressionfactor = 5;
	int ERI_w = 3840;
	int ERI_h = 2048;
	float We = wee;
	float Het = hett;
	float Heb = Het;
	float R0R4 = ERI_w - 2 * We*compressionfactor;
	float R0R1 = ERI_h - 2 * Het*compressionfactor;
	float R0x = We * compressionfactor;
	float R0y = Het * compressionfactor;
	float mxrow = ERI_h - 1;
	float mxcol = ERI_w - 1;
	float distx, disty;
	cout << "mapx....start..........................." <<ERI_w<<" "<<ERI_h<< endl;


	for (int row = 0; row <= ERI_h; row++)
	{
		for (int col = 0; col <= ERI_w; col++)
		{

			if ((row > R0y && row < R0y + R0R1) && (col > R0x && col < R0x + R0R4))
			{
				disty = row - (R0y - Het);
				distx = col - (R0x - We);
				colMap[row][col] = distx;
				rowMap[row][col] = disty;
			}
			else
			{
				float d = R0x - col;
				//	cout << "d" << " " << nonuniformDrev[d] << endl;
				float Dx = R0x - nonuniformDrev[d];
				if (col < R0x)
				{
					float x1 = col * (float)R0y / (float)R0x;
					float x2 = mxrow - (float)col*(mxrow - (R0y + R0R1)) / (float)R0x;
					if ((row >= x1) && (row <= x2))
					{
						float dx = col;
						float dy = row;
						float Cx = Dx;
						float Ex = Dx;
						float My = dx * (float)R0y / (float)R0x;
						float Cy = Cx * (float)R0y / (float)R0x;
						float Ny = mxrow - (float)dx*(mxrow - R0y - R0R1) / (float)R0x;
						float Ey = mxrow - (float)Ex*(mxrow - R0y - R0R1) / (float)R0x;
						float Dy = Ey - (float)((Ey - Cy)*(Ny - dy)) / (float)(Ny - My);
						float distx = Dx - (R0x - We);
						float disty = Dy - (R0y - Het);
						colMap[row][col] = distx;
						rowMap[row][col] = disty;
					}
				} //end region 1

				//region 3
				if (col >= R0x + R0R4)
				{
					float d = col - (R0x + R0R4);
					float dx = col;
					//cout << d << " " << nonuniformDrev[d] << endl;
					float Dx = nonuniformDrev[d] + (R0x + R0R4);;
					//cout << "decode col2: " << col << " Dx: " << (Dx - (R0x - We)-We-R0R4) << endl;
					float y1 = (float)R0y*(mxcol - col) / (float)(mxcol - R0x - R0R4);
					float y2 = mxrow + (float)(col - mxcol)*(mxrow - (R0y + R0R1)) / (float)(mxcol - (R0x + R0R4));

					if ((row >= y1) && (row <= y2))
					{
						float dy = row;
						float Cx = Dx;
						float Ex = Dx;
						float My = (float)R0y*(mxcol - dx) / (float)(mxcol - R0x - R0R4);
						float Cy = (float)R0y*(mxcol - Dx) / (float)(mxcol - R0x - R0R4);
						float Ny = mxrow + (float)(dx - mxcol)*(mxrow - (R0y + R0R1)) / (float)(mxcol - (R0x + R0R4));
						float Ey = mxrow + (float)(Dx - mxcol)*(mxrow - (R0y + R0R1)) / (float)(mxcol - (R0x + R0R4));
						float Dy = Ey - (float)((Ey - Cy)*(Ny - dy)) / (float)(Ny - My);
						float distx = Dx - (R0x - We);
						float disty = Dy - (R0y - Het);
						colMap[row][col] = distx;
						rowMap[row][col] = disty;

					}
				}  //end region 3

				if (row <= R0y)
				{
					float d = R0y - row;
					float dy = row;
					//cout << d <<" "<<nonuniformDrev2.size()<< endl;
					//cout << "d" << " " << nonuniformDrev[d] << endl;
					float Dy = R0y - nonuniformDrev2[d];
					//cout << "decode r2: " << row << " Dy: " << (Dy - (R0y - Het) ) << endl;

					float y1 = col * (float)R0y / (float)R0x;
					float y2 = (float)R0y*(mxcol - col) / (float)(mxcol - R0x - R0R4);

					if ((row <= y1) && (row <= y2))
					{
						float dx = col;
						float Cy = Dy;
						float Ey = Dy;
						float Mx = dy * (float)R0x / (float)R0y;
						float Cx = Dy * (float)R0x / (float)R0y;;
						float Nx = mxcol - dy * (float)(mxcol - R0x - R0R4) / (float)R0y;
						float Ex = mxcol - Dy * (float)(mxcol - R0x - R0R4) / (float)R0y;
						float Dx = Ex - (float)((Ex - Cx)*(Nx - dx)) / (float)(Nx - Mx);
						float distx = Dx - (R0x - We);
						float disty = Dy - (R0y - Het);
						colMap[row][col] = distx;
						rowMap[row][col] = disty;

					}
				}  //end region 2

				//region 4: bottom
				if (row >= R0y + R0R1)
				{
					float d = row - (R0y + R0R1);
					//cout << d << " " << nonuniformDrev[d] << endl;
					float Dy = nonuniformDrev2[d] + (R0y + R0R1);
					float x1 = (mxrow - row)*(float)R0x / (mxrow - (R0y + R0R1));
					float x2 = mxcol + (row - mxrow)*(float)(mxcol - (R0x + R0R4)) / (mxrow - (R0y + R0R1));

					if ((col >= x1) && (col <= x2))
					{
						float dx = col;
						float dy = row;

						float Cy = Dy;
						float Ey = Dy;
						float Mx = (mxrow - dy)*(float)R0x / (mxrow - (R0y + R0R1));
						float Cx = (mxrow - Dy)*(float)R0x / (mxrow - (R0y + R0R1));
						float Nx = mxcol + (dy - mxrow)*(float)(mxcol - (R0x + R0R4)) / (mxrow - (R0y + R0R1));
						float Ex = mxcol + (Dy - mxrow)*(float)(mxcol - (R0x + R0R4)) / (mxrow - (R0y + R0R1));
						float Dx = Ex - (float)((Ex - Cx)*(Nx - dx)) / (float)(Nx - Mx);
						float distx = Dx - (R0x - We);
						float disty = Dy - (R0y - Het) - 1;
						colMap[row][col] = distx;
						rowMap[row][col] = disty;
					}

				}  //end region 4

			}

		}

	}
	cout << "mapx....end..........................." << endl;
}


void Path::CRERI2convOptimized(Mat & CRERI, Mat & convPixels,  PPC camera1, int angle, int fi)
{
	int ERI_w = 3840;
	int ERI_h = 2048;
	float We = wee;
	float Het = hett;
	float Heb = Het;
	float R0R4 = ERI_w - 2 * We*5;
	float R0R1 = ERI_h - 2 * Het*5;
	float R0x = We * 5;
	float R0y = Het * 5;
	int mxrow = ERI_h - 1;
	int mxcol = ERI_w - 1;
	int pixelI, pixelJ = 0;
	float prevdx = 0;
	float currentdx = 0;
	updateReriCs(angle);
	camera1.a = reriCS * camera1.a;
	camera1.b = reriCS * camera1.b;
	camera1.c = reriCS * camera1.c;

	for (int v = 0; v <= camera1.h; v++)
	{
		V3 p1 = camera1.b * v + camera1.c;
		for (int u = 0; u <= camera1.w; u++)
		{
			V3 p = camera1.a*u + p1;
			p = p.UnitVector();

			int col = eri.GetXYZ2LongitudeOptimized(p);

			int row= eri.GetXYZ2LatitudeOptimized(p[1]);

			float disty = rowMap[row][col];
			float distx = colMap[row][col];
            if (disty >= CRERI.rows)
            {
                disty = CRERI.rows - 1;
            }
            if (distx >= CRERI.cols)
            {
                distx = CRERI.cols - 1;
            }
            if (disty < 0)
            {
                disty = 0;
            }
            if (distx < 0)
            {
                distx = 0;
            }
            convPixels.at<Vec3b>(v, u) = CRERI.at<Vec3b>((int)disty, (int)distx);
			//bilinearinterpolation(convPixels, CRERI, v, u, disty, distx);
            //__android_log_print(ANDROID_LOG_INFO, "myTag","entered CRERIconvopt.........................................>>");
		}
	}

   // __android_log_print(ANDROID_LOG_INFO, "myTag","exited CRERIconvopt.........................................>>");

}
//instead returning we should pass address, pass all of these using pointer

Mat Path::CRERI2Conv(Mat & CRERI, int compressionfactor, PPC camera1, PPC refcam)
{

	float compressionfactorX = compressionfactor;
	float compressionfactorY = (float)compressionfactor / (float)1;
	int ERI_w = 3840;
	int ERI_h = 2048;
	float We = wee;
	float Het = hett;
	float Heb = Het;
	float R0R4 = ERI_w - 2 * We*compressionfactorX;
	float R0R1 = ERI_h - 2 * Het*compressionfactorY;
	float R0x = We * compressionfactor;
	float R0y = Het * compressionfactor;


	/**********************************Recreate Mapping******************************************/

	vector <float> nonuniformDrev;
	vector <float > nonuniformDrowrev;

	for (int col = 0; col < R0x; col++)
	{
		float j = (float)col / (float)R0x;
		float a = 1 - (float)1 / (float)compressionfactorX - 0;
		float b = (1 - a);
		a = a * (-1);
		float quad_out = R0x * (b - (float)sqrt(b*b - 4 * a*j)) / (float)(2 * a*compressionfactorX);
		nonuniformDrev.push_back(quad_out);

	}
	vector <float> nonuniformDrev2;
	vector <float > nonuniformDrowrev2;
	for (int row = 0; row < R0y; row++)
	{
		float j = (float)row / (float)R0y;
		float a = 1 - ((float)1 / (float)compressionfactorY);
		float b = (1 - a);
		a = a * (-1);
		float quad_out = R0y * (b - (float)sqrt(b*b - 4 * a*j)) / (float)(2 * a*compressionfactorY);
		nonuniformDrev2.push_back(quad_out);
	}

	int mxrow = ERI_h - 1;
	int mxcol = ERI_w - 1;
	/***************************Region 01: left *****************************************/

	// build local coordinate system of RERI
	V3 xaxis = refcam.a.UnitVector();
	V3 yaxis =refcam.b.UnitVector()*-1.0f;
	V3 zaxis = xaxis ^ yaxis;
	M33 reriCS;
	reriCS[0] = xaxis;
	reriCS[1] = yaxis;
	reriCS[2] = zaxis;

	ERI eri(ERI_w, ERI_h);
	int pixelI, pixelJ = 0;
	Mat convPixels(camera1.h, camera1.w, CRERI.type()); //do not create it here, pass address in the parameter

	float prevdx = 0;
	float currentdx = 0;
	float a = (float)1 / (float)compressionfactor - 1;
	float b = 1 - a;
	float vto = abs((float)(2 * a) / (float)(2 * a + b));
	float vtin = abs((float)(2 * a) / (float)(b));

	for (int v = 0; v < camera1.h; v++)
	{
		for (int u = 0; u < camera1.w; u++)
		{
			V3 p = camera1.UnprojectPixel(u, v, 1.0f);
			p = reriCS * p;  //xopt rate camera   , restrict p between -1 to 1 of its x, y and z
			int col = eri.Lon2PixJ(eri.GetXYZ2Longitude(p)); //xxOpt: get float and use bilinear
			int	row = eri.Lat2PixI(eri.GetXYZ2Latitude(p));

			if ((row > R0y && row < R0y + R0R1) && (col > R0x && col < R0x + R0R4))
			{
				convPixels.at<Vec3b>(v, u) = CRERI.at<Vec3b>(row - (R0y - Het), col - (R0x - We)); //xxxOpt: this has to be bilinear interpolation; first change row col as float
			}
			else
			{
				float d = R0x - col;
				float Dx = R0x - nonuniformDrev[d]; //avoid discretization here, call with rox-colf directly;

				if (col < R0x)
				{
					float x1 = col * (float)R0y / (float)R0x;
					float x2 = mxrow - (float)col*(mxrow - (R0y + R0R1)) / (float)R0x;
					if ((row > x1) && (row < x2))
					{
						float dx = col;
						float dy = row;
						float Cx = Dx;
						float Ex = Dx;
						float My = dx * (float)R0y / (float)R0x;
						float Cy = Cx * (float)R0y / (float)R0x;
						float Ny = mxrow - (float)dx*(mxrow - R0y - R0R1) / (float)R0x;
						float Ey = mxrow - (float)Ex*(mxrow - R0y - R0R1) / (float)R0x;
						float Dy = Ey - (float)((Ey - Cy)*(Ny - dy)) / (float)(Ny - My);
						float distx = Dx - (R0x - We);
						float disty = Dy - (R0y - Het);

						float dist = distx - prevdx;
						prevdx = distx;


						if (disty >= CRERI.rows)
						{
							disty = CRERI.rows - 1;
						}
						if (distx >= CRERI.cols)
						{
							distx = CRERI.cols - 1;
						}
						if (disty < 0)
						{
							disty = 0;
						}
						if (distx < 0)
						{
							distx = 0;
						}
						bilinearinterpolation(convPixels, CRERI, v, u, disty, distx);
						float t = (float)d / (float)R0x;
						float vt = abs((float)2 * a / (float)(2 * a*t + b));

					}
				} //end region 1


				if (col > R0x + R0R4)
				{
					float d = col - (R0x + R0R4);
					float dx = col;
					float Dx = nonuniformDrev[d] + (R0x + R0R4);;
					float y1 = (float)R0y*(mxcol - col) / (float)(mxcol - R0x - R0R4);
					float y2 = mxrow + (float)(col - mxcol)*(mxrow - (R0y + R0R1)) / (float)(mxcol - (R0x + R0R4));

					if ((row > y1) && (row < y2))
					{
						float dy = row;
						float Cx = Dx;
						float Ex = Dx;
						float My = (float)R0y*(mxcol - dx) / (float)(mxcol - R0x - R0R4);
						float Cy = (float)R0y*(mxcol - Dx) / (float)(mxcol - R0x - R0R4);
						float Ny = mxrow + (float)(dx - mxcol)*(mxrow - (R0y + R0R1)) / (float)(mxcol - (R0x + R0R4));
						float Ey = mxrow + (float)(Dx - mxcol)*(mxrow - (R0y + R0R1)) / (float)(mxcol - (R0x + R0R4));
						float Dy = Ey - (float)((Ey - Cy)*(Ny - dy)) / (float)(Ny - My);
						float distx = Dx - (R0x - We);
						float disty = Dy - (R0y - Het);
						if (disty >= CRERI.rows)
						{
							disty = CRERI.rows - 1;
						}
						if (distx >= CRERI.cols)
						{
							distx = CRERI.cols - 1;
						}
						if (disty < 0)
						{
							disty = 0;
						}
						if (distx < 0)
						{
							distx = 0;
						}
						bilinearinterpolation(convPixels, CRERI, v, u, disty, distx);
						float t = (float)d / (float)R0x;
						float vt = abs((float)2 * a / (float)(2 * a*t + b));
					}
				}  //end region 3

				//region 2: top

				if (row < R0y)
				{
					float d = R0y - row;
					float dy = row;
					float Dy = R0y - nonuniformDrev2[d];

					float y1 = col * (float)R0y / (float)R0x;
					float y2 = (float)R0y*(mxcol - col) / (float)(mxcol - R0x - R0R4);

					if ((row < y1) && (row < y2))
					{
						float dx = col;
						float Cy = Dy;
						float Ey = Dy;
						float Mx = dy * (float)R0x / (float)R0y;
						float Cx = Dy * (float)R0x / (float)R0y;;
						float Nx = mxcol - dy * (float)(mxcol - R0x - R0R4) / (float)R0y;
						float Ex = mxcol - Dy * (float)(mxcol - R0x - R0R4) / (float)R0y;
						float Dx = Ex - (float)((Ex - Cx)*(Nx - dx)) / (float)(Nx - Mx);
						float distx = Dx - (R0x - We);
						float disty = Dy - (R0y - Het);

						if (disty > CRERI.rows - 1)
						{
							disty = CRERI.rows - 1;
						}

						if (distx > CRERI.cols - 1)
						{
							distx = CRERI.cols - 1;
						}
						if (disty < 0)
						{
							disty = 0;
						}
						if (distx < 0)
						{
							distx = 0;
						}

						bilinearinterpolation(convPixels, CRERI, v, u, disty, distx);
						float t = (float)d / (float)R0y;
						float vt = abs((float)2 * a / (float)(2 * a*t + b));

					}
				}  //end region 2

				if (row > R0y + R0R1)
				{
					float d = row - (R0y + R0R1);
					float Dy = nonuniformDrev2[d] + (R0y + R0R1);
					float x1 = (mxrow - row)*(float)R0x / (mxrow - (R0y + R0R1));
					float x2 = mxcol + (row - mxrow)*(float)(mxcol - (R0x + R0R4)) / (mxrow - (R0y + R0R1));

					if ((col > x1) && (col < x2))
					{
						float dx = col;
						float dy = row;
						float Cy = Dy;
						float Ey = Dy;
						float Mx = (mxrow - dy)*(float)R0x / (mxrow - (R0y + R0R1));
						float Cx = (mxrow - Dy)*(float)R0x / (mxrow - (R0y + R0R1));
						float Nx = mxcol + (dy - mxrow)*(float)(mxcol - (R0x + R0R4)) / (mxrow - (R0y + R0R1));
						float Ex = mxcol + (Dy - mxrow)*(float)(mxcol - (R0x + R0R4)) / (mxrow - (R0y + R0R1));
						float Dx = Ex - (float)((Ex - Cx)*(Nx - dx)) / (float)(Nx - Mx);
						float distx = Dx - (R0x - We);
						float disty = Dy - (R0y - Het) - 1;
						if (disty >= CRERI.rows)
						{
							disty = CRERI.rows - 1;
						}
						if (distx >= CRERI.cols)
						{
							distx = CRERI.cols - 1;
						}
						if (disty < 0)
						{
							disty = 0;
						}
						if (distx < 0)
						{
							distx = 0;
						}
						bilinearinterpolation(convPixels, CRERI, v, u, disty, distx);
						float t = (float)d / (float)R0y;
						float vt = abs((float)2 * a / (float)(2 * a*t + b));

					}

				}  //end region 4

			}
		}

	}

	return convPixels;

}


void Path::bilinearinterpolation(Mat &output, Mat &source, int Orow, int Ocol, float uf, float vf)
{

    if (vf < (source.cols - 1.5) && vf> 0.5 &&  uf > 0.5 && uf < (source.rows - 1.5))
    {
        int u0 = (int)(uf - 0.5f);
        int v0 = (int)(vf - 0.5f);
        float dx = vf - 0.5f - v0;
        float dy = uf - 0.5f - u0;

        Vec3b color = (1 - dx)*(1 - dy)* source.at<Vec3b>(u0, v0) + (dx)*(1 - dy)*source.at<Vec3b>(u0, v0 + 1) +
                      dx * dy*source.at<Vec3b>(u0 + 1, v0 + 1) + (1 - dx)*dy*source.at<Vec3b>(u0 + 1, v0);

        output.at<Vec3b>(Orow, Ocol) = color;
    }
    else
    {

        if (vf >= source.cols)
        {
            vf = source.cols - 1;
        }
        if (uf >= source.rows)
        {
            uf = source.rows - 1;
        }
        if (vf < 0)
        {
            vf = 0;
        }
        if (uf < 0)
        {
            uf = 0;
        }
        output.at<Vec3b>(Orow, Ocol) = source.at<Vec3b>((int)uf, (int)vf);
    }
}


