//#include"curl/curl.h"
#include"path.h"
#include"image.h"
#include<stdio.h>
#include<math.h>
#include <stdlib.h>
#include"config.h"
#include "ERI.h"
#include <chrono>
#include "pixelCalculation.h"

using namespace std;
using namespace std::chrono;
#include <C:\opencv\build\include\opencv2\videoio.hpp>
#include <C:\opencv\build\include\opencv2\opencv.hpp>
#include <C:\opencv\build\include\opencv2\core\core.hpp>
#include <C:\opencv\build\include\opencv2\highgui\highgui.hpp>
using namespace cv;
vector <float> nonuniformDrev;
vector <float > nonuniformDrowrev;
vector <float> nonuniformDrev2;
vector <float > nonuniformDrowrev2;
float colMap[3850][3850];//ch
float rowMap[3850][3850];//ch
float samplingRate[2500][4000];//ch


Path::Path() {
	print("One Path created" << endl);
}

Path::~Path() {
	print("One Path deleted" << endl);
}



void Path::AppendCamera(PPC newCam, int framesN)
{
	cams.push_back(newCam);
	segmentFramesN.push_back(framesN);
}


void Path::LoadHMDTrackingData(char* filename, PPC ppc0)
{

	ifstream  file(filename);
	if (!file)
	{
		print("error: can't open file: " << filename << endl);
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
		V3 v(v2, v3, v1);		
		camera1.RotateAboutAxisThroughEye(v, theta2 * 2 * 180 / PI);
		AppendCamera(camera1, 2);	
		//cout << camera1.GetVD() << endl;
				
	}
	//STOP;
	print("read: " << cams.size() << " cameras from: " << filename << endl);
	
}

int Path::GetCamIndex(int fi, int fps, int segi) {

	int ret = segi;
	while (tstamps[ret] <= (float)(fi) / (float)(fps))
	{
		ret++;
		if (ret >= tstamps.size())
		{
			cerr << "Reached the end of the path without finding HMD pos for frame" << endl;
			waitKey(10000);
			exit(0);  //eita on thakbe.

		}

	}

	return ret - 1;

}
//cam index for playback at 3s, 7s etc...
int Path::GetCamIndexUsingTime(float time_sec, int last_cam_index) {

	int ret = last_cam_index;
	while (tstamps[ret] <= time_sec)
	{
		ret++;
		if (ret >= tstamps.size())
		{
			cerr << "Reached the end of the path without finding HMD pos for frame" << endl;
			waitKey(10000);
			exit(0);  //eita on thakbe.

		}

	}

	return ret - 1;

}


void Path::DrawBoundinigBoxframe(Mat frame, PPC camera, float& pxl, float& pxr, float&pxu, float &pxd)
{
	Mat newERI(frame.rows, frame.cols, frame.type());

	ERI eri(frame.cols, frame.rows);
	int pixelI, pixelJ = 0;
	float PxL = eri.w;
	float PxR = 0;
	float PxU = eri.h;
	float PxD = 0;


	for (int v = 0; v < camera.h; v++)
	{
		int u = 0;
		eri.EachPixelConv2ERI(camera, u, v, pixelI, pixelJ);
		if (pixelJ < PxL)
			PxL = pixelJ;

		u = camera.w - 1;
		eri.EachPixelConv2ERI(camera, u, v, pixelI, pixelJ);
		if (pixelJ > PxR)
			PxR = pixelJ;

	}
	for (int u = 0; u < camera.w; u++)
	{
		int v = 0;
		eri.EachPixelConv2ERI(camera, u, v, pixelI, pixelJ);
		if (pixelI < PxU)
			PxU = pixelI;

		v = camera.h - 1;
		eri.EachPixelConv2ERI(camera, u, v, pixelI, pixelJ);
		if (pixelI > PxD)
			PxD = pixelI;

	}

	//cout << PxL << " " << PxR << " " << PxU << " " << PxD << endl;


	//Rect RectangleToDraw(left, up, (PxR - PxL), (PxD - PxU));
	//rectangle(newERI, RectangleToDraw.tl(), RectangleToDraw.br(), Scalar(255, 0, 0), 2, 8, 0);

	//float var[4];
	pxl = PxL;
	pxr = PxR;
	pxu = PxU;
	pxd = PxD;

	return;

}

Mat Path::EncodeNewNonLinV2(Mat frame, struct var * var1, PPC camera1, PPC encodeRefPPC, int compressionfactor)
{
	float compressionfactorY = (float)compressionfactor;
	float compressionfactorX = compressionfactor;

	if (frame.empty())
	{
		cout << "Error loading image" << endl;

	}

	/********************Get middle point and create create bounding box*******************/
	/*****************Ret is used to check weather the image has right before left (In case of image splited between end and start of the ERI***********************/
	//float hfov = 110.0f;
	//PPC camera(hfov, 1168, 657);  //size property should be exactly same with camera1.
	//same as camera1
	V3 pb = encodeRefPPC.GetVD();
	V3 pa = camera1.GetVD();

	// build local coordinate system of RERI
	V3 xaxis = camera1.a.UnitVector();
	V3 yaxis = camera1.b.UnitVector()*-1.0f;
	V3 zaxis = xaxis ^ yaxis;
	M33 reriCS;
	reriCS[0] = xaxis;
	reriCS[1] = yaxis;
	reriCS[2] = zaxis;

	Mat midcorrectedmat(frame.rows, frame.cols, frame.type());
	RotateXYaxisERI2RERI(frame, midcorrectedmat, pb, pa, reriCS);
	img_write("./Image/temp/midcorrected.PNG", midcorrectedmat);

	/*
	namedWindow("sample", WINDOW_NORMAL);
	resizeWindow("sample", 800, 400);
	imshow("sample", midcorrectedmat);
	waitKey(1000); */

	float pxl; float pxr; float pxu; float pxd;
	DrawBoundinigBoxframe(midcorrectedmat, encodeRefPPC, pxl, pxr, pxu, pxd);

	float PxL = pxl;
	float PxR = pxr;
	float PxU = pxu;
	float PxD = pxd;


	//cout << PxL <<" "<<PxR<<" "<<PxU<<" "<<PxD<< endl;

	//system("pause");
	int R0R1 = PxD - PxU;
	int R0R4 = PxR - PxL;

	float R0x = ((float)frame.cols / 2.0f - (float)R0R4 / 2.0f);
	float R0y = ((float)frame.rows / 2.0f - (float)(R0R1) / 2.0f);
	int Q0x = 0;
	int Q0y = frame.rows;

	float We = (float)(frame.cols - R0R4) / (float)(2 * compressionfactorX);
	float Het = (float)(frame.rows - R0R1) / (float)(2 * compressionfactorY);
	float Heb = Het;

	//print("frame size:"<<frame.size() << endl);
	//print("ror1: " << R0R1 << " ror4:" << R0R4 << " rox:" << R0x << " roy:" << R0y << " we:" << We << " het:" << Het << " heb:" << Heb);

	Mat distortedframemat = Mat::zeros((Het + R0R1 + Heb), (2 * We + R0R4), frame.type());
	Mat tmp = midcorrectedmat(Rect(R0x, R0y, R0R4, R0R1));  //midcorrected one is CRERI

	/*********************** create a red outline********************************************/
	Rect RectangleToDraw(0, 0, tmp.cols, tmp.rows);
	//rectangle(tmp, RectangleToDraw.tl(), RectangleToDraw.br(), Scalar(255,0, 255), 2, 8, 0);
	tmp.copyTo(distortedframemat(Rect(We, Het, R0R4, R0R1)));     //copy the bounding box and paste in the distortedframemat image. 

	/*
		Q(0,0)
			___________________________________________________________________________________________________
			|\												/ \													|
			|	\	C										 |(R0y)												|
			|		\		P()								 |													|
			|		:	\	 ________________________________|_______________________________					|
			|		:		|\								 |	   / \						|					|
			|		:		|	\	M						 |		|Het					|					|
			|		:		|		\						 |		|						|					|
			|		:		|		.	\	R0(R0x,R0y)		 |		|	   R4(R0x+R4R1,R0y)    					|					|
			|		:    	|		.		\_______________\_/____\_/_____										|					|
			|	<------(R0x)-------------->	|								|				|					|
			|		:		|		.		|								|				|					|
			|		:		|<........We..>	|								|				|					|
			|		:       |		.	    |								|				|					|
			D(Dx,Dy):<------cfactor*d--->/	|								|				|					|
			|		:		|		./		|								|				|					|
			|		:		|	/	.<--d-->(R0x, Roy+R0R1)	     			|				|					|
			|		:	  /	|		.	 R1 |______________________________	|									|					|
			|		:/		|		.		/												|					|
			|		:		|		.	/													|					|
			|		:		|		/														|					|
			|		:		|	/	N														|					|
			|		:		|/______________________________________________________________|					|
			|		:																							|
			|		:	/																						|
			|		/E																							|
			|	/																								|
			|/__________________________________________________________________________________________________|



	Q: original midcorreccted ERI image
	P: distorted (compressed) ERI image
	R: bounding box of the visualized ERI pixels
	R0x=horizontal distance between original ERI and bounding box
	R0y= same for vertical axis
	We= thickness of padding in x axis (x distance between P and R)
	Het=thickness of upper region
	Heb=thickness of lower region
	MN=line being transformed
	EC=line which MN will be transfered



	*/

	/*****************Encode each of the four region: left, top, right, bottom********************/
	int mxdrow = distortedframemat.rows - 1;
	int mxdcol = distortedframemat.cols - 1;
	int mxcrow = midcorrectedmat.rows - 1;
	int mxccol = midcorrectedmat.cols - 1;

	//to calculated nonuniform distance and then correct it by rotatiing the array/vector//

	/*
	In this region with increasing the array comes in oppossite direction. means when col increases
	d actually increases in inverse direction.  Thats why the
	array needs to be inversed. For this reason we created next two loop. First one
	calculate inverted d and second one correct it.
	f(d)=axx+bx+c; cond1: f(both_edge_of_dist_ERI)= both_edge_of_big_ERI; f'(d_inner_edge_of_dist)=1
	to make sampling constant with the inner rectangle
	the equation becomes b=1; a=(100-10)/10*10. Where d=0 maps to 0 and d=10 maps to 100:
	means edges are at 0,10 for distmat and 0,100 for originalmat
	*/

	vector <float> nonuniformD;
	vector <float > nonuniformDtemp;
	vector <float > nonuniformDrowcorrected;
	//to understand logic for this two loop, print them somewhere and see with 
	//increasing col quad_out keep increasing the gap. this d here is not our real d.
	//read d starts from the R0R1 and go towards QQ1. So we need to turn the array top to bottom

	Mat quality(frame.rows, frame.cols, frame.type());

	for (int col = 0; col < We; col++)
	{
		float j = (float)col / (float)We;
		float a = ((float)1 / (float)compressionfactorX) - 1;
		float v = (a)*j*j + (1 - a)*j;
		float quad_out = v * We*compressionfactorX;
		nonuniformD.push_back(quad_out);
	//	cout <<"col: "<<col<<" nonuniformD: "<< quad_out << endl;

	}

	for (int i = 0; i < nonuniformD.size(); i++)
	{
		//cout << We * compressionfactorX << " " << nonuniformD[i] << endl;
		float x = We * compressionfactorX - nonuniformD[i];
		//cout <<i<<" "<< x << endl;
		nonuniformDtemp.push_back(x);

	}  //*/


	for (int i = nonuniformDtemp.size() - 1; i > -1; i--)
	{
		//cout << nonuniformDtemp[i] << endl;
		nonuniformDrowcorrected.push_back(nonuniformDtemp[i]);
		//cout<<"corrected:" << nonuniformDrowcorrected[nonuniformDtemp.size()-1-i]<<endl;
	}
	/*
	for (int i = 0; i < nonuniformD.size(); i++)
	{
		//cout << nonuniformDrowcorrected[i]<<endl;

	}  //*/

	vector <float> nonuniformD2;
	vector<float> nonuniformD2temp;
	vector <float > nonuniformDrowcorrected2;


	for (int row = 0; row < Het; row++)
	{
		float j = (float)row / (float)Het;
		float a = ((float)1 / (float)compressionfactorY) - 1;
		float v = (a)*j*j + (1 - a)*j;
		float quad_out = v * Het*compressionfactorY;
		nonuniformD2.push_back(quad_out);
		//cout << row << " " << quad_out << endl;
	}

	for (int i = 0; i < nonuniformD2.size(); i++)
	{
		float y = Het * compressionfactorY - nonuniformD2[i];
		nonuniformD2temp.push_back(y);

	}  //*/	

	for (int i = nonuniformD2.size() - 1; i > -1; i--)
	{
		nonuniformDrowcorrected2.push_back(nonuniformD2temp[i]);
	}
	/*
	for (int i = 0; i < nonuniformD2.size(); i++)
	{
		cout << nonuniformDrowcorrected2[i]<<endl;

	}  //*/


	

	//////////////
	for (int col = 0; col < We; col++)
	{
		float Dx = nonuniformD[col];
		//print("Dx and We: " << Dx <<" "<<We<< endl);
		for (int row = 0; row < distortedframemat.rows; row++)
		{
			float xx = (float)col / (float)row;
			float yy = (float)We / (float)Het;
			if (xx < yy && col < (float)(We*(float)(mxdrow - row) / (float)(Heb)))
			{
				//region L this one using the technique of using distance to find a point with a slope m//

				//float Dx = R0x - nonuniformDrowcorrected[col];
				float Cx = Dx;
				float Ex = Dx;
				float Cy = Cx * (float)R0y / (float)R0x;

				//use equation to find E,C, M,N. We know their x distance, put that in line equation to find y distance//

				float My = ((R0x - We) + col) * (float)R0y / ((float)R0x);
				float Ny = mxcrow + (((R0x - We) + col) * (float)(R0y + R0R1 - mxcrow)) / (float)R0x;
				float Ey = mxcrow + (Ex * (float)(R0y + R0R1 - mxcrow)) / (float)R0x;
				float Dy = Cy + (float)(Ey - Cy)*(float)(row + R0y - Het - My) / (float)(Ny - My);
				//cout << Dx << " " << Dy << endl;
				distortedframemat.at<Vec3b>(row, col) = midcorrectedmat.at<Vec3b>(Dy, Dx);
				bilinearinterpolation(distortedframemat, midcorrectedmat, row, col, Dy, Dx);
			}
		}
	}


	//print("loop3" << endl);
	for (int col = We + R0R4; col < distortedframemat.cols; col++)
	{
		float d = col - We - R0R4;
		float Dx = R0x + R0R4 + nonuniformDrowcorrected[d];
		//print("Dx2:" << Dx-(R0x + R0R4) << endl);
		for (int row = 0; row < distortedframemat.rows; row++)
		{
			float xx = Heb * ((float)(col - mxdcol) / (float)We) + mxdrow;

			if ((row < xx) && (row > (float)(Het*(float)(mxdcol - col) / (float)(We))))
			{
				//3rd region. This region is alligned with our quadratic equation means
				// when col increase d also increase in same direction. So we dont need
				// additional work we did in region 1	
				float Cy = R0y * (float)(mxccol - Dx) / (float)(mxccol - R0x - R0R4);
				float My = R0y * (float)(mxccol - (R0x - We + col)) / (float)(mxccol - R0x - R0R4);
				float Ey = mxcrow + (Dx - mxccol)*(float)(mxcrow - R0y - R0R1) / (float)(mxccol - R0x - R0R4);
				float Ny = mxcrow + ((R0x - We + col) - mxccol)*(float)(mxcrow - R0y - R0R1) / (float)(mxccol - R0x - R0R4);
				float Dy = Cy + (Ey - Cy)*(float)(row + R0y - Het - My) / (float)(Ny - My);
				distortedframemat.at<Vec3b>(row, col) = midcorrectedmat.at<Vec3b>(Dy, Dx);
				bilinearinterpolation(distortedframemat, midcorrectedmat, row, col, Dy, Dx);

			}
		}

	}


	//print("loop2" << endl);

	//same logic for quadratic equation of region one


	for (int row = 0; row < Het; row++)
	{
		float Dy = nonuniformD2[row];
		//print("Dy:" << Dy << endl);


		for (int col = 0; col < distortedframemat.cols; col++)
		{
			float xx = (float)col / (float)row;
			float yy = (float)We / (float)Het;

			if (xx > yy && row < (float)(Het*(float)(mxdcol - col) / (float)(We)))
			{
				//Next two commented line for uniform sampling
				//float d = Het-row;  //get distance between current line with the base line			
				//float Dy = R0y - compressionfactor * d;  //Get Dy by multiplying d with factor, this line in orig ERI represent line d of distorted ERI


				float Cx = Dy * (float)R0x / ((float)R0y);  //Cy==Dy

				//use equation to find E,C, M,N. We know their x distance, put that in line equation to find y distance//

				float Mx = ((R0y - Het) + row) * (float)R0x / ((float)R0y);
				float Nx = mxccol + ((R0y - Het) + row) * (float)(R0x + R0R4 - mxccol) / (float)R0y;
				float Ex = mxccol + Dy * (float)(R0x + R0R4 - mxccol) / (float)R0y;
				float Dx = Cx + (float)(Ex - Cx)*(float)(col + R0x - We - Mx) / (float)(Nx - Mx);
				distortedframemat.at<Vec3b>(row, col) = midcorrectedmat.at<Vec3b>(Dy, Dx);
				//print(Dy << ","<<Dx << endl);
				bilinearinterpolation(distortedframemat, midcorrectedmat, row, col, Dy, Dx);
			}
		}
	}


	//print("loop4" << endl);
	for (int row = Het + R0R1; row < distortedframemat.rows; row++)
	{
		float d = row - (Het + R0R1);
		float Dy = (R0y + R0R1) + nonuniformDrowcorrected2[d];
		//print("Dy2:" << Dy << endl);

		for (int col = 0; col < distortedframemat.cols; col++)
		{
			float xx = Heb * ((float)(col - mxdcol) / (float)We) + mxdrow;
			if (row > xx && col > (float)(We*(float)(mxdrow - row) / (float)(Heb)))
			{
				//4th region has same logic as second region for quad//

				float Cx = R0x * ((float)(Dy - mxcrow) / (float)(R0y + R0R1 - mxcrow));
				float Mx = R0x * ((float)(row + R0y - Het - mxcrow) / (float)(R0y + R0R1 - mxcrow));
				float Nx = mxccol + (mxcrow - (row + R0y - Het))*((float)(mxccol - R0x - R0R4) / (float)(R0y + R0R1 - mxcrow));
				float Ex = mxccol + (mxcrow - Dy)*((float)(mxccol - R0x - R0R4) / (float)(R0y + R0R1 - mxcrow));
				float Dx = Cx + (float)(Ex - Cx)*(float)(col + R0x - We - Mx) / (float)(Nx - Mx);
				//distortedframemat.at<Vec3b>(row, col) = midcorrectedmat.at<Vec3b>(Dy, Dx);
				bilinearinterpolation(distortedframemat, midcorrectedmat, row, col, Dy, Dx);
			}
		}
	}
	//print("loop4" << endl);
	//float overallcompressionfactor = 100 * distortedframemat.rows*distortedframemat.cols / (midcorrectedmat.rows*midcorrectedmat.cols);

	//OverlayImage(&midcorrectedmat, &distortedframemat, Point((R0x-We), (R0y-Het)));
	//eri.VisualizeNeededPixels(frame, &cams[segi]);
	(var1)->colN = frame.cols;
	(var1)->rowN = frame.rows;
	(var1)->We = We;
	(var1)->Het = Het;//*/

	//print(We<<","<< R0x<<","<< Het<<" "<< Heb<<" "<< R0y<<endl);
	return distortedframemat;

}//mainloop of End of Encoding



void Path::nonUniformListInit(float var[10]) 
{
	cout << ".......NU.......... "<< endl;
	int compressionfactor = 5;
	float R0x = var[2] * 5;
	float R0y = var[3] * 5;
	//cout << var[2] << " " << var[3] << endl;
	for (int col = 0; col <R0x+1; col++)
	{
		float j = (float)col / (float)R0x;
		float a = 1 - (float)1 / (float)compressionfactor - 0;
		float b = (1 - a);
		a = a * (-1);
		float quad_out = R0x * (b - (float)sqrt(b*b - 4 * a*j)) / (float)(2 * a*compressionfactor);
		//cout << a << " " << b << " " << quad_out << endl;
		nonuniformDrev.push_back(quad_out);
	}

	for (int row = 0; row < R0y+1; row++)
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



void Path::mapx(float var[10])
{
	int compressionfactor = 5;
	int ERI_w = var[0];
	int ERI_h = var[1];
	float We = var[2];
	float Het = var[3];
	float Heb = Het;
	float R0R4 = ERI_w - 2 * We*compressionfactor;
	float R0R1 = ERI_h - 2 * Het*compressionfactor;
	float R0x = We * compressionfactor;
	float R0y = Het * compressionfactor;
	float mxrow = ERI_h - 1;
	float mxcol = ERI_w - 1;
	float distx, disty;
	cout << "mapx....start..........................." <<ERI_w<<" "<<ERI_h<< endl;
	vector<float>nonUniformReg3;
	vector<float>nonUniformReg4;
	for (int col = 0; col < R0x + 1; col++)
	{
		//cout << nonuniformDrev[col]<<" " << nonuniformDrev[nonuniformDrev.size() - col - 1] << endl;
		nonUniformReg3.push_back(nonuniformDrev[nonuniformDrev.size()-col-1]);
	}

	//cout << "---------------------------------------------------------" << endl;

	for (int row = 0; row < R0y + 1; row++)
	{
		//cout << nonuniformDrev2[row] << " " << nonuniformDrev2[nonuniformDrev2.size() - row - 1] << endl;
		nonUniformReg4.push_back(nonuniformDrev2[nonuniformDrev2.size() - row - 1]);
	}

	for (int row = 0; row < ERI_h; row++)
	{
		
		for (int col = 0; col < ERI_w; col++)
		{
			

			if ((row >= R0y && row <= R0y + R0R1) && (col >= R0x && col <= R0x + R0R4))
			{
				//cout << "dR: " << row << " col: " << col << " diff: " << 111 << endl;
				disty = row - (R0y - Het);
				distx = col - (R0x - We);
				colMap[row][col] = distx;
				rowMap[row][col] = disty;	
				samplingRate[row][col] = 100.0f;
			}
			
			else
			{				
				
				if (col < R0x)
				{
					float d = R0x - col;
					float Dx = R0x - nonuniformDrev[d];

					float sampleDiff1 = nonUniformReg3[col] - nonUniformReg3[col+ 1];

					float x1 = col * (float)R0y / (float)R0x;
					float x2 = mxrow - (float)col*(mxrow - (R0y + R0R1)) / (float)R0x;
					if ((row > x1) && (row < x2))
					{
						//cout << "dL: " << row << " col: " << col << " diff: " << sampleDiff << endl;
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
						samplingRate[row][col] = sampleDiff1*100.0f;
					}
				} //end region 1

				//region 3
				if (col > R0x + R0R4)
				{
					float d = col - (R0x + R0R4);
					float dx = col;
					
					float sampleDiff3 = nonuniformDrev[d] - nonuniformDrev[d - 1];
				
					float Dx = nonuniformDrev[d] + (R0x + R0R4);;
					float y1 = (float)R0y*(mxcol - col) / (float)(mxcol - R0x - R0R4);
					float y2 = mxrow + (float)(col - mxcol)*(mxrow - (R0y + R0R1)) / (float)(mxcol - (R0x + R0R4));

					if ((row > y1) && (row < y2))
					{
						//cout << "dR: " << row << " col: " << col << " diff: " << sampleDiff << endl;
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
						samplingRate[row][col] = sampleDiff3*100.0f;

					}
				}  //end region 3

				if (row < R0y)
				{
					float d = R0y - row;
					float dy = row;
				
					float Dy = R0y - nonuniformDrev2[d];
					
					
					float y1 = col * (float)R0y / (float)R0x;
					float y2 = (float)R0y*(mxcol - col) / (float)(mxcol - R0x - R0R4);

					if ((row < y1) && (row < y2))
					{
						float sampleDiff2 = nonUniformReg4[row] - nonUniformReg4[row + 1];
						//cout << "dU: " << row << " col: " << col << " diff: " << sampleDiff << endl;
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
						samplingRate[row][col] = sampleDiff2*100.0f;

					}
				}  //end region 2

				//region 4: bottom
				if (row > R0y + R0R1)
				{
					float d = row - (R0y + R0R1);					
				
					float Dy = nonuniformDrev2[d] + (R0y + R0R1);
					float x1 = (mxrow - row)*(float)R0x / (mxrow - (R0y + R0R1));
					float x2 = mxcol + (row - mxrow)*(float)(mxcol - (R0x + R0R4)) / (mxrow - (R0y + R0R1));

					if ((col > x1) && (col < x2))
					{
						float sampleDiff4 = nonuniformDrev2[d] - nonuniformDrev2[d - 1];
						//cout << "dD: " << row << " col: " << col << " diff: " << sampleDiff << endl;
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
						samplingRate[row][col] = sampleDiff4*100.0f;
					}

				}  //end region 4

			}
			
		}

	}

	samplingRate[0][0] = 11.0f;

	
	for (int col = 0; col < ERI_w; col++)
	{
		//cout << "row: " << 1000 << "col: " << col << "SR: " << samplingRate[1000][col] << endl;
	}



	for (int row = 0; row < ERI_h; row++)
	{
		
		//cout << "row: " << row << "col: " << 1500 << "SR: " << samplingRate[row][1500] << endl;
		
	}
	
	cout << "mapx....end..........................." << endl;
}


void Path:: calculateAllSamplingRateOverCreri(float var[10])
{
	int compressionfactor = 5;

	int ERI_w = var[0];
	int ERI_h = var[1];
	float We = var[2];
	float Het = var[3];
	float Heb = Het;
	float R0R4 = ERI_w - 2 * We*compressionfactor;
	float R0R1 = ERI_h - 2 * Het*compressionfactor;
	float R0x = We * compressionfactor;
	float R0y = Het * compressionfactor;
	int mxCrow = ERI_h - 1;
	int mxCcol = ERI_w - 1;

	int mxDcol = 2 * We + R0R4 - 1;
	int mxDrow = 2 * Het + R0R1 - 1;

	cout << R0R4 << " " << R0R1 << " "<<mxDrow<<" "<<mxDcol<<endl;

	vector <float> nonUniformCol;
	vector <float > nonUniformRow;


	for (int col = 0; col <= We; col++)
	{
		float j = (float)col / (float)We;
		float a = ((float)1 / (float)compressionfactor) - 1;
		float v = (a)*j*j + (1 - a)*j;
		int quad_out = round(v * We*compressionfactor);
		nonUniformCol.push_back(quad_out);
		//cout << "col: " << col << " nonuniformCol: " << quad_out << endl;

	}

	vector <float> nonuniformD2;
	vector<float> nonuniformD2temp;
	vector <float > nonuniformDrowcorrected2;


	for (int row = 0; row <= Het; row++)
	{
		float j = (float)row / (float)Het;
		float a = ((float)1 / (float)compressionfactor) - 1;
		float v = (a)*j*j + (1 - a)*j;
		int quad_out = round(v * Het*compressionfactor);
		nonUniformRow.push_back(quad_out);
		//cout <<"row: "<< row << "nonUniformRow " << quad_out << endl;
	}


	Mat output = Mat::zeros(mxDrow + 1, mxDcol + 1, 16);



	for (int row = 0; row < mxDrow + 1; row++)
	{
		for (int col = 0; col < mxDcol + 1; col++)
		{

			if ((col >= We && col <= We + R0R4) && (row >= Het && row <= Het + R0R1))
			{
				samplingRate[row][col] = 1;

			}
		}


	}


	for (int row = 0; row < int(Het); row++)
	{
		int diff = nonUniformRow[row + 1] - nonUniformRow[row];
		for (int col = 0; col < mxDcol + 1; col++)
		{
			float xx = (float)col / (float)row;
			float yy = (float)We / (float)Het;
			if (xx > yy && row < (float)(Het*(float)(mxDcol - col) / (float)(We)))
			{
				samplingRate[row][col] = diff;

			}
		}
	}


	//////////////
	for (int col = 0; col < int(We); col++)
	{
		int diff = (nonUniformCol[col + 1] - nonUniformCol[col]);

		for (int row = 0; row < mxDrow + 1; row++)
		{
			float xx = (float)col / (float)row;
			float yy = (float)We / (float)Het;
			if (xx < yy && col < (float)(We*(float)(mxDrow - row) / (float)(Heb)))
			{
				samplingRate[row][col] = diff;
			}
		}
	}


	//print("loop3" << endl);
	for (int col = We + R0R4; col < mxDcol + 1; col++)
	{
		float d = mxDcol + 1 - col;
		int diff = nonUniformCol[d] - nonUniformCol[d - 1];

		for (int row = 0; row < mxDrow + 1; row++)
		{
			float xx = Heb * ((float)(col - mxDcol) / (float)We) + mxDrow;

			if ((row < xx) && (row > (float)(Het*(float)(mxDcol - col) / (float)(We))))
			{
				samplingRate[row][col] = diff;


				//	cout<<"r3: d"<<d<<" "<<row << " " << col << " " << samplingRate[row][col] << endl;
			}
		}

	}



	//print("loop4" << endl);
	for (int row = int(Het + R0R1); row < mxDrow + 1; row++)
	{
		float d = mxDrow + 1 - row;
		int diff = nonUniformRow[d - 1] - nonUniformRow[d - 2];


		for (int col = 0; col < ERI_w; col++)
		{
			float xx = Heb * ((float)(col - mxDcol) / (float)We) + mxDrow;
			if (row > xx && col > (float)(We*(float)(mxDrow - row) / (float)(Heb)))
			{
				samplingRate[row][col] = diff;

			}
		}
	}





	for (int row = 0; row < mxDrow + 1; row++)
	{
		for (int col = 0; col < mxDcol + 1; col++)
		{
			Vec3b color(15 * samplingRate[row][col], 0, 0);

			output.at<Vec3b>(row, col) = color;
			if (row==0 && col == 0)
			{
				int diff = nonUniformRow[1] - nonUniformRow[0];
				samplingRate[row][col] = 1;//diff;ch it supposed to be the lowest, but there is a bug somewhere.
				
				
			}
	

		}

	}
	/*
	for (int row = ERI_h / 2; row < ERI_h / 2 + 1; row++)
	{
		for (int col = 0; col < ERI_w; col++)
		{
			cout << "row: " << row << "col: " << col << "SR: " << samplingRate[row][col] << endl;
		}
	}

	for (int row = 0; row < ERI_h; row++)
	{
		for (int col = ERI_w/2; col < ERI_w/2+1; col++)
		{
			cout << "row: " << row << "col: " << col << "SR: " << samplingRate[row][col] << endl;
		}
	}
	STOP;
	*/
	cout << "sampling rate daone.." << endl;

}


void Path::CRERI2ConvOptimized(Mat CRERI, float var[10], ERI& eri, M33 & reriCS, Mat & convPixels, int compressionfactor, PPC camera1, PPC refcam)
{
	int ERI_w = var[0];
	int ERI_h = var[1];
	float We = var[2];
	float Het = var[3];	
	float Heb = Het;
	float R0R4 = ERI_w - 2 * We*compressionfactor;
	float R0R1 = ERI_h - 2 * Het*compressionfactor;
	float R0x = We * compressionfactor;
	float R0y = Het * compressionfactor;
	int mxrow = ERI_h - 1;
	int mxcol = ERI_w - 1;
	int pixelI, pixelJ = 0;
	float prevdx = 0;
	float currentdx = 0;
	//auto start = std::chrono::high_resolution_clock::now();
	camera1.a = reriCS * camera1.a;
	camera1.b = reriCS * camera1.b;
	camera1.c = reriCS * camera1.c;
	for (int v = 0; v < camera1.h; v++)
	{
		V3 p1 = camera1.b * v + camera1.c;
		for (int u = 0; u < camera1.w; u++)
		{				
			V3 p = camera1.a*u + p1;		
			p = p.UnitVector();			
			int col = eri.GetXYZ2LongitudeOptimized(p);
			int row= eri.GetXYZ2LatitudeOptimized(p[1]);		
			float CRERIrow = rowMap[row][col];
			float CRERIcol= colMap[row][col];	
			bilinearinterpolation(convPixels, CRERI, v, u, CRERIrow, CRERIcol);
		}
	}	
	return;
}
//auto finish = std::chrono::high_resolution_clock::now();
//std::chrono::duration<double> elapsed = finish - start;
//cout << "Time................=" << elapsed.count() * 1000 << endl;	

void Path::CRERI2ConvOptimizedWithSamplingRate(Mat CRERI, float var[10], ERI& eri, M33& reriCS, Mat& convPixels, Mat& samplingPixels, int compressionfactor, PPC camera1, PPC refcam)
{
	int ERI_w = var[0];
	int ERI_h = var[1];
	float We = var[2];
	float Het = var[3];
	float Heb = Het;
	float R0R4 = ERI_w - 2 * We * compressionfactor;
	float R0R1 = ERI_h - 2 * Het * compressionfactor;
	float R0x = We * compressionfactor;
	float R0y = Het * compressionfactor;
	int mxrow = ERI_h - 1;
	int mxcol = ERI_w - 1;
	int pixelI, pixelJ = 0;
	float prevdx = 0;
	float currentdx = 0;
	//auto start = std::chrono::high_resolution_clock::now();
	camera1.a = reriCS * camera1.a;
	camera1.b = reriCS * camera1.b;
	camera1.c = reriCS * camera1.c;
	for (int v = 0; v < camera1.h; v++)
	{
		V3 p1 = camera1.b * v + camera1.c;
		for (int u = 0; u < camera1.w; u++)
		{
			V3 p = camera1.a * u + p1;
			p = p.UnitVector();
			int col = eri.GetXYZ2LongitudeOptimized(p);
			int row = eri.GetXYZ2LatitudeOptimized(p[1]);
			if (row >= ERI_w) { row = ERI_w - 1; }//ch
			if (col >= ERI_w) { col = ERI_w - 1; }//ch
			if (row < 0) { row = 0; }
			if (col < 0) { col = 0; }
			float CRERIrow = rowMap[row][col];
			float CRERIcol = colMap[row][col];
			bilinearinterpolation(convPixels, CRERI, v, u, CRERIrow, CRERIcol);
			samplingPixels.at<Vec3b>(v, u) = Vec3b(samplingRate[(int)(CRERIrow)][(int)(CRERIcol)], 0, 0);
			if (samplingRate[(int)(CRERIrow)][(int)(CRERIcol)] == 0)
			{
				cout << "u,v: " << u << " " << v << CRERIrow << " creri row col " << CRERIcol << endl;
			}

		}
	}
	return;
}

void Path::CRERI2ConvOptimizedWithSamplingRateVec(Mat CRERI, float var[10], ERI& eri, M33& reriCS, Mat& convPixels, vector <float>& srVec, int compressionfactor, PPC camera1, PPC refcam)
{
	int ERI_w = var[0];
	int ERI_h = var[1];
	float We = var[2];
	float Het = var[3];
	float Heb = Het;
	float R0R4 = ERI_w - 2 * We * compressionfactor;
	float R0R1 = ERI_h - 2 * Het * compressionfactor;
	float R0x = We * compressionfactor;
	float R0y = Het * compressionfactor;
	int mxrow = ERI_h - 1;
	int mxcol = ERI_w - 1;
	int pixelI, pixelJ = 0;
	float prevdx = 0;
	float currentdx = 0;
	//auto start = std::chrono::high_resolution_clock::now();
	camera1.a = reriCS * camera1.a;
	camera1.b = reriCS * camera1.b;
	camera1.c = reriCS * camera1.c;
	long totalSR = 0;
	for (int v = 0; v < camera1.h; v++)
	{
		V3 p1 = camera1.b * v + camera1.c;
		for (int u = 0; u < camera1.w; u++)
		{
			V3 p = camera1.a * u + p1;
			p = p.UnitVector();
			int col = eri.GetXYZ2LongitudeOptimized(p);
			int row = eri.GetXYZ2LatitudeOptimized(p[1]);
			if (row >= ERI_w) { row = ERI_w - 1; }//ch
			if (col >= ERI_w) { col = ERI_w - 1; }//ch
			if (row < 0) { row = 0; }
			if (col < 0) { col = 0; }
			float CRERIrow = rowMap[row][col];
			float CRERIcol = colMap[row][col];
			bilinearinterpolation(convPixels, CRERI, v, u, CRERIrow, CRERIcol);
			totalSR = totalSR + samplingRate[row][col];
			//cout <<"SR: "<< samplingRate[row][col] << endl;

		}
	}
	float localAvg = totalSR / (camera1.h * camera1.w);
	srVec.push_back(localAvg);
	cout << "localAvg: " << localAvg << endl;
	return;
}


Mat Path::CRERI2Conv(Mat CRERI, float var[10], int compressionfactor, PPC camera1, PPC refcam, Mat& qual, struct samplingvar * var1)
{
	float compressionfactorX = compressionfactor;
	float compressionfactorY = (float)compressionfactor / (float)1;
	int ERI_w = var[0];
	int ERI_h = var[1];
	float We = var[2];
	float Het = var[3];
	float Heb = Het;
	float R0R4 = ERI_w - 2 * We*compressionfactorX;
	float R0R1 = ERI_h - 2 * Het*compressionfactorY;
	float R0x = We * compressionfactor;
	float R0y = Het * compressionfactor;


	/**********************************Recreate Mapping******************************************/

	vector <float> nonuniformDrev;
	vector <float > nonuniformDrowrev;
	//to understand logic for this two loop, print them somewhere and see with 
	//increasing col quad_out keep increasing the gap. this d here is not our real d.
	//read d starts from the R0R1 and go towards QQ1. So we need to turn the array top to bottom


	for (int col = 0; col < R0x; col++)
	{
		float j = (float)col / (float)R0x;
		float a = 1 - (float)1 / (float)compressionfactorX - 0;
		float b = (1 - a);
		a = a * (-1);
		int quad_out = R0x * (b - (float)sqrt(b*b - 4 * a*j)) / (float)(2 * a*compressionfactorX);
		cout<<"col 0 to rox: j= " << j << " col= " << col << " out= " << quad_out << endl;
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
		int quad_out = R0y * (b - (float)sqrt(b*b - 4 * a*j)) / (float)(2 * a*compressionfactorY);
		cout << "rw 0 to roy: j= " << j << " row= " << row << " out= " << quad_out << endl;
		nonuniformDrev2.push_back(quad_out);
	}
	STOP;
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
	Mat convPixels(camera1.h, camera1.w, CRERI.type());

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
			p = reriCS * p;
			int col = eri.Lon2PixJ(eri.GetXYZ2Longitude(p));
			int	row = eri.Lat2PixI(eri.GetXYZ2Latitude(p));

			if ((row > R0y && row < R0y + R0R1) && (col > R0x && col < R0x + R0R4))
			{
				convPixels.at<Vec3b>(v, u) = CRERI.at<Vec3b>(row - (R0y - Het), col - (R0x - We));
				qual.at<float>(v, u) = vtin;
				//cout << v << " u: " << u << " row: " << row << " col: " << col << " " << 10 * vtin << endl;
			}
			else
			{
				float d = R0x - col;
				//cout << "d: " << d << endl;
				float Dx = R0x - nonuniformDrev[d];
				//cout << "decode col_r1: " << col << " Dx: " << (Dx - (R0x - We)) << endl;

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

						//	cout <<"diff: "<< dist <<"current: "<<distx<< " prev: " << prevdx << endl;

						if (disty >= CRERI.rows)
						{
							//cout << distx << " " << disty << " " << CRERI.size() << endl;
							disty = CRERI.rows - 1;
						}
						if (distx >= CRERI.cols)
						{
							//cout << distx << " " << disty << " " << CRERI.size() << endl;
							distx = CRERI.cols - 1;
						}
						if (disty < 0)
						{
							//cout << distx << " " << disty << " " << CRERI.size() << endl;
							disty = 0;
						}
						if (distx < 0)
						{
							//cout << distx << " " << disty << " " << CRERI.size() << endl;
							distx = 0;
						}
						//convPixels.at<Vec3b>(v, u) = CRERI.at<Vec3b>(disty, distx);
						bilinearinterpolation(convPixels, CRERI, v, u, disty, distx);
						float t = (float)d / (float)R0x;
						float vt = abs((float)2 * a / (float)(2 * a*t + b));
						//cout << v << " u: " << u << " row: " << row << " col: " << col << " t: " << t << " " << 10 * vt << endl;
						qual.at<float>(v, u) = vt;


					}
				} //end region 1

				//print("dloop1" << endl);
				//region 3


				if (col > R0x + R0R4)
				{
					float d = col - (R0x + R0R4);
					float dx = col;
					float Dx = nonuniformDrev[d] + (R0x + R0R4);;
					//cout << "decode col2: " << col << " Dx: " << (Dx - (R0x - We)-We-R0R4) << endl;					
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
							//cout << distx << " " << disty << " " << CRERI.size() << endl;
							disty = CRERI.rows - 1;
						}
						if (distx >= CRERI.cols)
						{
							//cout << distx << " " << disty << " " << CRERI.size() << endl;
							distx = CRERI.cols - 1;
						}
						if (disty < 0)
						{
							//cout << distx << " " << disty << " " << CRERI.size() << endl;
							disty = 0;
						}
						if (distx < 0)
						{
							//cout << distx << " " << disty << " " << CRERI.size() << endl;
							distx = 0;
						}
						//print(distx << " " << disty << " " << CRERI.size() << endl);						
						//bilinearinterpolation(decodedframe, CRERI, row, col, disty, distx);
						//convPixels.at<Vec3b>(v, u) = CRERI.at<Vec3b>(disty, distx);
						bilinearinterpolation(convPixels, CRERI, v, u, disty, distx);
						float t = (float)d / (float)R0x;
						float vt = abs((float)2 * a / (float)(2 * a*t + b));
						//cout << v << " u: " << u << " row: " << row << " col: " << col << " t: " << t << " " << 10 * vt << endl;
						qual.at<float>(v, u) = vt;
					}
				}  //end region 3


				//	print("dloop2" << endl);

					//region 2: top

				if (row < R0y)
				{
					float d = R0y - row;
					float dy = row;
					//cout << d <<" "<<nonuniformDrev2.size()<< endl;
					float Dy = R0y - nonuniformDrev2[d];
					//cout << "decode r2: " << row << " Dy: " << (Dy - (R0y - Het) ) << endl;

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

						//decodedframe.at<Vec3b>(row, col) = CRERI.at<Vec3b>(disty, distx);
						//bilinearinterpolation(decodedframe, CRERI, row, col, disty, distx);
						//convPixels.at<Vec3b>(v, u) = CRERI.at<Vec3b>(disty, distx);
						bilinearinterpolation(convPixels, CRERI, v, u, disty, distx);
						float t = (float)d / (float)R0y;
						float vt = abs((float)2 * a / (float)(2 * a*t + b));
						//cout << v << " u: " << u << " row: " << row << " col: " << col << " t: " << t << " " << 10 * vt << endl;
						qual.at<float>(v, u) = vt;

					}
				}  //end region 2

				//print("dloop3" << endl);
				//region 4: bottom

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
							//cout << distx << " " << disty << " " << CRERI.size() << endl;
							disty = CRERI.rows - 1;
						}
						if (distx >= CRERI.cols)
						{
							//cout << distx << " " << disty << " " << CRERI.size() << endl;
							distx = CRERI.cols - 1;
						}
						if (disty < 0)
						{
							//cout << distx << " " << disty << " " << CRERI.size() << endl;
							disty = 0;
						}
						if (distx < 0)
						{
							//cout << distx << " " << disty << " " << CRERI.size() << endl;
							distx = 0;
						}
						//decodedframe.at<Vec3b>(row, col) = CRERI.at<Vec3b>(disty, distx);
						bilinearinterpolation(convPixels, CRERI, v, u, disty, distx);
						//convPixels.at<Vec3b>(v, u) = CRERI.at<Vec3b>(disty, distx);
						float t = (float)d / (float)R0y;
						float vt = abs((float)2 * a / (float)(2 * a*t + b));
						//cout <<v<<" u: "<<u<<" row: "<<row<<" col: "<<col<<" t: "<< t << " " << 10*vt << endl;
						qual.at<float>(v, u) = vt;

					}

				}  //end region 4
				//	print("dloop4" << endl);			

			}
		}

	}

	float minv = 0;
	float sum = 0;
	int number = 0;
	float bb;

	//cout << vtin << " " << vto << endl;

	for (int i = 0; i < camera1.h; i++)
	{
		for (int jj = 0; jj < camera1.w; jj++)
		{
			bb = qual.at<float>(i, jj);
			if (bb == 0) { bb = vtin; }
			if (bb > minv)
			{
				minv = (float)vtin / (float)(bb);
				//cout <<jj<<" bb: "<< bb<<" minv: "<< minv << endl;
			}
			sum = sum + bb;
			number++;
		}
	}
	float average = (float)(vtin*number) / (float)(sum);
	//cout << "avg: "<<average << " min: " << minv << endl;


	/*
	ofstream output("./Video/encodingtest/qual.txt");
	output << qual << endl;
	output.close();
	/*
	namedWindow("sample1", WINDOW_NORMAL);
	resizeWindow("sample1", 800, 400);
	imshow("qual", qual);
	waitKey(1000); */
	var1->vtin = vtin;
	var1->vto = vto;
	var1->avg = average;
	var1->min = minv;

	return convPixels;

}

Mat Path::CRERI2ConvQual(Mat CRERI, float var[10], int compressionfactor, PPC camera1, PPC refcam, Mat& qual, struct samplingvar * var1)
{
	float compressionfactorX = compressionfactor;
	float compressionfactorY = (float)compressionfactor / (float)1;
	int ERI_w = var[0];
	int ERI_h = var[1];
	float We = var[2];
	float Het = var[3];
	float Heb = Het;
	float R0R4 = ERI_w - 2 * We*compressionfactorX;
	float R0R1 = ERI_h - 2 * Het*compressionfactorY;
	float R0x = We * compressionfactor;
	float R0y = Het * compressionfactor;


	/**********************************Recreate Mapping******************************************/

	vector <float> nonuniformDrev;
	vector <float > nonuniformDrowrev;
	//to understand logic for this two loop, print them somewhere and see with 
	//increasing col quad_out keep increasing the gap. this d here is not our real d.
	//read d starts from the R0R1 and go towards QQ1. So we need to turn the array top to bottom


	for (int col = 0; col < R0x; col++)
	{
		float j = (float)col / (float)R0x;
		float a = 1 - (float)1 / (float)compressionfactorX - 0;
		float b = (1 - a);
		a = a * (-1);
		float quad_out = R0x * (b - (float)sqrt(b*b - 4 * a*j)) / (float)(2 * a*compressionfactorX);
		//cout << "col 0 to rox: j= " << j << " col= " << col << " out= " << quad_out << endl;
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
		//cout << "rw 0 to roy: j= " << j << " row= " << row << " out= " << quad_out << endl;
		nonuniformDrev2.push_back(quad_out);
	}
	
	int mxrow = ERI_h - 1;
	int mxcol = ERI_w - 1;
	/***************************Region 01: left *****************************************/

	// build local coordinate system of RERI
	V3 xaxis = refcam.a.UnitVector();
	V3 yaxis = refcam.b.UnitVector()*-1.0f;
	V3 zaxis = xaxis ^ yaxis;
	M33 reriCS;
	reriCS[0] = xaxis;
	reriCS[1] = yaxis;
	reriCS[2] = zaxis;



	ERI eri(ERI_w, ERI_h);
	int pixelI, pixelJ = 0;
	Mat convPixels(camera1.h, camera1.w, CRERI.type());

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
			p = reriCS * p;
			int col = eri.Lon2PixJ(eri.GetXYZ2Longitude(p));
			int	row = eri.Lat2PixI(eri.GetXYZ2Latitude(p));

			if ((row > R0y && row < R0y + R0R1) && (col > R0x && col < R0x + R0R4))
			{
				qual.at<float>(v, u) = vtin;
			//	cout <<"vin "<< v << " u: " << u << " row: " << row << " col: " << col << " " << 10 * vtin << endl;
			}
			else
			{
				float d = R0x - col;
				float Dx = R0x - nonuniformDrev[d];
				//cout << "decode col_r1: " << col << " Dx: " << (Dx - (R0x - We)) << endl;

				if (col < R0x)
				{
					float x1 = col * (float)R0y / (float)R0x;
					float x2 = mxrow - (float)col*(mxrow - (R0y + R0R1)) / (float)R0x;
					if ((row > x1) && (row < x2))
					{
						
						float t = (float)d / (float)R0x;
						float vt = abs((float)2 * a / (float)(2 * a*t + b));
						//cout<<"vl: " << v << " u: " << u << " row: " << row << " col: " << col << " t: " << t << " " << 10 * vt << endl;
						qual.at<float>(v, u) = vt;


					}
				} //end region 1

				//print("dloop1" << endl);
				//region 3


				if (col > R0x + R0R4)
				{
					float d = col - (R0x + R0R4);
					float dx = col;
					float Dx = nonuniformDrev[d] + (R0x + R0R4);;
					//cout << "decode col2: " << col << " Dx: " << (Dx - (R0x - We)-We-R0R4) << endl;					
					float y1 = (float)R0y*(mxcol - col) / (float)(mxcol - R0x - R0R4);
					float y2 = mxrow + (float)(col - mxcol)*(mxrow - (R0y + R0R1)) / (float)(mxcol - (R0x + R0R4));

					if ((row > y1) && (row < y2))
					{
						
						float t = (float)d / (float)R0x;
						float vt = abs((float)2 * a / (float)(2 * a*t + b));
						//cout <<"vr: "<< v << " u: " << u << " row: " << row << " col: " << col << " t: " << t << " " << 10 * vt << endl;
						qual.at<float>(v, u) = vt;
					}
				}  //end region 3


				//	print("dloop2" << endl);

					//region 2: top

				if (row < R0y)
				{
					float d = R0y - row;
					float dy = row;
					//cout << d <<" "<<nonuniformDrev2.size()<< endl;
					float Dy = R0y - nonuniformDrev2[d];
					//cout << "decode r2: " << row << " Dy: " << (Dy - (R0y - Het) ) << endl;

					float y1 = col * (float)R0y / (float)R0x;
					float y2 = (float)R0y*(mxcol - col) / (float)(mxcol - R0x - R0R4);

					if ((row < y1) && (row < y2))
					{
						
						float t = (float)d / (float)R0y;
						float vt = abs((float)2 * a / (float)(2 * a*t + b));
						//cout <<"vt: "<< v << " u: " << u << " row: " << row << " col: " << col << " t: " << t << " " << 10 * vt << endl;
						qual.at<float>(v, u) = vt;

					}
				}  //end region 2

				//print("dloop3" << endl);
				//region 4: bottom

				if (row > R0y + R0R1)
				{
					float d = row - (R0y + R0R1);
					float Dy = nonuniformDrev2[d] + (R0y + R0R1);
					float x1 = (mxrow - row)*(float)R0x / (mxrow - (R0y + R0R1));
					float x2 = mxcol + (row - mxrow)*(float)(mxcol - (R0x + R0R4)) / (mxrow - (R0y + R0R1));

					if ((col > x1) && (col < x2))
					{
						
						float t = (float)d / (float)R0y;
						float vt = abs((float)2 * a / (float)(2 * a*t + b));
						//cout <<"vb: "<<v<<" u: "<<u<<" row: "<<row<<" col: "<<col<<" t: "<< t << " " << 10*vt << endl;
						qual.at<float>(v, u) = vt;

					}

				}  //end region 4
				//	print("dloop4" << endl);			

			}
		}

	}

	float minv = 0;
	float sum = 0;
	int number = 0;
	float bb;

	cout << vtin << " " << vto << endl;

	for (int i = 0; i < camera1.h; i++)
	{
		for (int jj = 0; jj < camera1.w; jj++)
		{
			bb = qual.at<float>(i, jj);
			if (bb == 0) { bb = vtin; }
			if (bb > minv)
			{
				minv = (float)vtin / (float)(bb);
				//cout <<jj<<" bb: "<< bb<<" minv: "<< minv << endl;
			}
			sum = sum + bb;
			number++;
		}
	}
	float average = (float)(vtin*number) / (float)(sum);
	cout << "avg: "<<average << " min: " << minv << endl;


	
	ofstream output("./Video/encodingtest/qual.txt");
	output << qual << endl;
	output.close();
	
	namedWindow("sample1", WINDOW_NORMAL);
	resizeWindow("sample1", 800, 400);
	imshow("qual", qual);
	waitKey(100); 
	var1->vtin = vtin;
	var1->vto = vto;
	var1->avg = average;
	var1->min = minv;

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
			//print(vf << " " << uf << " " << source.size() << endl);
		}
		if (uf >= source.rows)
		{
			uf = source.rows - 1;
			//print(vf << " " << uf << " " << source.size() << endl);
		}
		if (vf < 0)
		{
			vf = 0;
			//print(vf << " " << uf << " " << source.size() << endl);
		}
		if (uf < 0)
		{
			uf = 0;
			//print(vf << " " << uf << " " << source.size() << endl);
		}		
		output.at<Vec3b>(Orow, Ocol) = source.at<Vec3b>((int)uf, (int)vf);

	}
}

void Path::RotateXYaxisERI2RERI(Mat originERI, Mat& newERI, V3 directionbefore, V3 directionaftertilt, M33 reriCS)
{

	ERI oERI(originERI.cols, originERI.rows);
	V3 p = directionbefore;
	V3 p1 = directionaftertilt;
	V3 a = p.UnitVector();
	V3 b = p1.UnitVector();
	float m = p * p1;
	V3 dir = (p1^p).UnitVector();
	float angle = ((float)180 / (float)PI)* acos(m);
	angle = angle;
	if (angle == 0)
	{
		newERI = originERI;
		return;
	}
	//cout << a << " " << b << " m: " << m<<" angle: " <<angle << endl;

	//system("pause");

	for (int j = 0; j < newERI.rows; j++)
	{
		for (int i = 0; i < newERI.cols; i++)
		{
			V3 q = oERI.Unproject(j, i);
			//cout << q << endl;
			//cout << reriCS << endl;
			q = reriCS.Inverted() * q;
			//cout << q << endl;
			//q = q.RotateThisVectorAboutDirection(dir, angle);
			int u = oERI.Lon2PixJ(oERI.GetXYZ2Longitude(q));
			int v = oERI.Lat2PixI(oERI.GetXYZ2Latitude(q));
			newERI.at<Vec3b>(j, i) = originERI.at<Vec3b>(v, u);

		}
	}


}



/******************Extra code and tests****************************/
/**********************************************/


void Path::PlayBackPathStillImage(Mat eriPixels, ERI eri, Mat convPixels) 
{
	//to play back every camera for one frame segmentFramesN[segi] needs to be 2
	//it does not reach last frame
	Mat erivis = cv::Mat::zeros(eri.h, eri.w, IMAGE_TYPE);
	//print(cams.size() - 1 << endl);	
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



void Path::PlayBackPathVideo(char* fname, Mat convPixels, int lastFrame)
{
	VideoCapture cap(fname);
	if (!cap.isOpened()) {
		cout << "Cannot open the video file: " <<fname<< endl;
		waitKey(100000);
		return;

	}	
	//ERI eri(cap.get(CAP_PROP_FRAME_WIDTH), cap.get(CAP_PROP_FRAME_HEIGHT));
	//Mat erivis = Mat::zeros(eri.h/5,eri.w/5, IMAGE_TYPE);
	//int fps = cap.get(CAP_PROP_FPS);
	
	float tstep = 0;
	int segi = 0;
	for (int fi=0; fi<=lastFrame; fi++)
	{  
		Mat frame;
		cap >> frame;	
		cout << fi << endl;
		imshow("image",frame);
		waitKey(20);
		if (frame.empty())
		{
			cout << "Can not read video frame no frame: "<<fname<< endl;
			waitKey(100000);
			return;
		}	
		//segi = GetCamIndex(fi, fps, segi);
		//eri.ERI2Conv(frame, convPixels, cams[segi]);	
		//V3 cameraDirection = cams[segi].GetVD();
		int tiltAngle = 20;
		//getChunkNametoReq(cameraDirection, tiltAngle);
	}
	
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
		print("done visualisation" << endl);
		waitKey(1);

	}

	//pixelcalc.PrintPixelInterval();
	pixelcalc.SaveIntervalTxt();
	print("done Interval saving" << endl);
}//most outer loop of playbackpathvideoopixelinterval





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
		
		if (fi == 1) {
			eri.getERIPixelsCount(erivis_noscale, &(cams[segi]), ERItotal);
		}
		

		if (fi%(fps*t) == 0) 
		{
			print("frame No: "<< fi << "chunk no: " << fi / (fps*t) << endl);
			for (int i = 0; i < m*n; i++)
			{
				
				if (Tilebitmap[i]==1)
				{
					totaltiles++;
				}
				print("tile: " << Tilebitmap[i] <<", Total-"<<totaltiles<< endl);
				Tilebitmap.at(i)=0;
			}
			waitKey(10);
		}

		
	}//outer for loop for all frame
	print("H: "<< eri.h<<" W: "<< eri.w<<" M: "<<m<<" N: "<<n<<" FPS: "<<fps<<" "<<" T: "<<t<<" Total Tiles: "<<totaltiles<< endl);
	double total_tile_pixel = eri.h*eri.w*fps*t*1UL / (m*n);
	total_tile_pixel = totaltiles * total_tile_pixel*1UL;
	double result = (ERItotal*lastFrame * 100UL /total_tile_pixel) ;
	//print("Total_tile_pixel: " << total_tile_pixel << endl);
	//print("Total requierd ERI pixel: " << ERItotal*lastFrame << endl);
	print("Required and supplied %: " << result << endl);

}

void Path::VDrotationAvg()
{	
	double total_angle=0;
	float angle=0;
	for (int i = 0; i < cams.size(); i++)
	{
		V3 cam1 = cams[i].GetVD();
		V3 cam2 = cams[i + 1].GetVD();
		print(cam1 <<"; " <<cam2<< endl);
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
		print("Angle: " << angle << " Total angle: " << total_angle << endl);
	
	}
	
	print("No of camera: " << cams.size() << "; Total Angle: " << total_angle << endl);
}

void Path::WriteH264(char* fname, int lastFrame, int codec)
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

	std::ostringstream oss;
	oss << fname;
	string filename = oss.str();

	VideoWriter writer;
	writer.set(VIDEOWRITER_PROP_QUALITY,20);
	writer.open(filename, codec, fps,Size(frame_width,frame_height), true);	
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

	writer.release();

}


void Path::WriteH264tiles(char* fname, int chunkN, int m, int n, int codec)
{
	
	int chunkD = 4;
	VideoCapture cap(fname);
	if (!cap.isOpened())
	{
		cout << "Cannot open the video file: " << fname << endl;
		waitKey(100000);
		return;

	}	
	int frame_width = cap.get(CAP_PROP_FRAME_WIDTH);
	int frame_height = cap.get(CAP_PROP_FRAME_HEIGHT);
	int fps = 30;


	vector <Mat> tileframes;


	for (int fi = 0; fi <= (chunkN) * fps * chunkD; fi++)
	{
		cout << fi << endl;
		Mat frame;
		cap >> frame;
		if (frame.empty())
		{
			cout << fi << endl;
			cout << "Can not read video frame: " << fname << endl;	
			break;
		}
		if (fi >= (chunkN - 1) * fps * chunkD)
		{
			tileframes.push_back(frame);

		}		
		
		
	}


	int Npx = frame_width / m;
	int Npy = frame_height / n;

	cout << Npx<<" " << Npy << endl;
	vector < vector< Mat > > image_array;

	high_resolution_clock::time_point t1 = high_resolution_clock::now();

	for (int i = 0; i < m*n; i++)
	{
		image_array.push_back(vector<Mat>());
	}

	int m1 = 0;

	for (int iy = 0; iy < n*Npy; iy += Npy)
	{	
		for (int ix = 0; ix < m*Npx; ix +=Npx)
		{
			
			for (int fi = 0; fi < tileframes.size(); fi++)
			{
				Mat frame = tileframes[fi];
				frame = frame(Range(iy, min(frame_height,iy + Npy)), Range(ix, min(frame_width,ix+Npx)));
				image_array[m1].push_back(frame);				
			}
			m1++;
		}
    }
	cout << "Writing videotile of: " << fname <<endl;
	
	for (int i = 0; i < m*n; i++)
	{	
		
		//cout << "Writing videotile: " << i << endl;
		std::ostringstream oss;
		oss << fname << "_" << chunkN <<"_"<<i<< ".avi";
		string filename = oss.str();
		VideoWriter writer1;
		cout <<"wrtingt->"<< filename << endl;
		writer1.open(filename, codec, fps, Size(Npx, Npy), true);

		if (!writer1.isOpened())
		{
			cerr << "Could not open the output video file for write\n";
			return;
		}
		for (int fi = 0; fi < tileframes.size(); fi++)
		{
			writer1.write(image_array[i][fi]);
		}
		writer1.release();
		
		
	}

	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(t2 - t1).count();
	cout << "time tiles for:" <<fname<<": "<< duration << endl;

	return;	
		
}


void Path::DrawBoundinigBox(char* fname, int lastFrame)
{	
	VideoCapture cap(fname);
	if (!cap.isOpened())
	{
		cout << "Can't open video: " << fname << endl;
		return;
	}


	for (int fi = 1; fi < lastFrame; fi++)
	{
		
		Mat frame;
		cap >> frame;
		if (frame.empty())
		{
			cout << "Reached beyond last frame. Fi=" << fi << "; LastFrame=" << lastFrame << endl;
			break;
		}
	
		Mat dualframe;
		Mat tripleframe;
		hconcat(frame, frame, dualframe);	
		
		ERI eri(cap.get(CAP_PROP_FRAME_WIDTH), cap.get(CAP_PROP_FRAME_HEIGHT));
		ERI eritriple(tripleframe.cols, tripleframe.rows);		
		int pixelI, pixelJ = 0;
		int PxL = eri.w;
		int PxR = 0;
		int PxU = eri.h;
		int PxD = 0;
		int segi = 0;
		int fps = cap.get(CAP_PROP_FPS);
		segi = GetCamIndex(fi, fps, segi);
		cams[segi].Pan(180);
		

		for (int v = 0; v < cams[segi].h; v++)
		{
			int u = 0;
			eri.EachPixelConv2ERI(cams[segi], u, v, pixelI, pixelJ);
			if (pixelJ < PxL)
				PxL = pixelJ;
			
			u = cams[segi].w - 1;
			eri.EachPixelConv2ERI(cams[segi], u, v, pixelI, pixelJ);
			if (pixelJ > PxR)
				PxR = pixelJ;			

		}


		for (int u = 0; u < cams[segi].w; u++)
		{
			int v = 0;
			eri.EachPixelConv2ERI(cams[segi], u, v, pixelI, pixelJ);
			if (pixelI < PxU)
				PxU = pixelI;

			v = cams[segi].h - 1;
			eri.EachPixelConv2ERI(cams[segi], u, v, pixelI, pixelJ);
			if (pixelI > PxD)
				PxD = pixelI;

		}
		int half;
		int midPx;
		int ret=eri.ERIPixelInsidePPC((int)(PxU + PxD) / 2,(int)(PxL+PxR)/2, &cams[segi]);
		
		int pxudmn = min(PxU, PxD);
		int pxudmx = max(PxU, PxD);
		Vec3b insidecolor(255, 0, 0);
		//cout << PxL << " " << PxR << " " << PxU << " " << PxD <<" "<<pxudmn<<" "<<pxudmx<< endl;;

		if (ret)
		{
			midPx = eri.w + (PxR + PxL) / 2;
			
			for (int u = PxL; u < PxR; u++)
			{
				for (int v = pxudmn; v <pxudmx; v++)
				{					
					dualframe.at<Vec3b>(v, u) = insidecolor;
					frame.at<Vec3b>(v, u) = insidecolor;
				}
			}
		}

		if (!ret)
		{
			
			midPx = eri.w + (eri.w + PxR + PxL) / 2;
			for (int u = max(PxR,PxL); u < eri.w; u++)
			{
				for (int v = pxudmn; v < pxudmx; v++)
				{					
					dualframe.at<Vec3b>(v, u) = insidecolor;
					frame.at<Vec3b>(v, u) = insidecolor;
				}

			}
			for (int u = 0; u < min(PxL,PxR); u++)
			{
				for (int v =pxudmn; v < pxudmx; v++)
				{					
					dualframe.at<Vec3b>(v, u + eri.w) = insidecolor;
					frame.at<Vec3b>(v, u) = insidecolor;
				}

			}
		}

		
		Mat midcorrectedmat;
		Mat mat1;
		Mat mat2;

		hconcat(frame, dualframe, tripleframe);;
		half = eri.w / 2;		
		mat1 = tripleframe.colRange((midPx - half), midPx);
		mat2 = tripleframe.colRange(midPx, (midPx + half));
		hconcat(mat1, mat2, midcorrectedmat);
		

		//eri.VisualizeNeededPixels(frame, &cams[segi]);
		namedWindow("sample", WINDOW_NORMAL);
		resizeWindow("sample", 600, 400);
		imshow("sample", frame);
		waitKey(1000);
		//imshow("sample", dualframe);
		//waitKey(1000);
		imshow("sample", midcorrectedmat);
		waitKey(1000);


	}//end of fi loop

}



void Path::DrawBoundinigBoxTemp(char* fname, int lastFrame)
{
	VideoCapture cap(fname);
	if (!cap.isOpened())
	{
		cout << "Can't open video: " << fname << endl;
		return;
	}


	for (int fi = 1; fi < lastFrame; fi++)
	{

		Mat frame;
		cap >> frame;
		if (frame.empty())
		{
			cout << "Reached beyond last frame. Fi=" << fi << "; LastFrame=" << lastFrame << endl;
			break;
		}

		Mat newERI(frame.rows,frame.cols, frame.type());
	
		ERI eri(cap.get(CAP_PROP_FRAME_WIDTH), cap.get(CAP_PROP_FRAME_HEIGHT));		
		int pixelI, pixelJ = 0;
		int PxL = eri.w;
		int PxR = 0;
		int PxU = eri.h;
		int PxD = 0;
		int segi = 0;
		int fps = cap.get(CAP_PROP_FPS);
		segi = GetCamIndex(fi, fps, segi);
		//cams[segi].Pan(180);
		PPC camera(110.0f, 800, 400);
		V3 pb=camera.GetVD();
		V3 pa = cams[segi].GetVD();

		// build local coordinate system of RERI
		V3 xaxis = cams[segi].a.UnitVector();
		V3 yaxis = cams[segi].b.UnitVector()*-1.0f;
		V3 zaxis = xaxis ^ yaxis;
		M33 reriCS;
		reriCS[0] = xaxis;
		reriCS[1] = yaxis;
		reriCS[2] = zaxis;

		
		RotateXYaxisERI2RERI(frame, newERI, pb,pa,reriCS);

		

		//cams[segi].PositionAndOrient(V3(0, 0, 0),pb, V3(0, 1, 0));
		cams[segi] = camera;
		for (int v = 0; v < cams[segi].h; v++)
		{
			int u = 0;
			eri.EachPixelConv2ERI(cams[segi], u, v, pixelI, pixelJ);
			if (pixelJ < PxL)
				PxL = pixelJ;

			u = cams[segi].w - 1;
			eri.EachPixelConv2ERI(cams[segi], u, v, pixelI, pixelJ);
			if (pixelJ > PxR)
				PxR = pixelJ;

		}
		for (int u = 0; u < cams[segi].w; u++)
		{
			int v = 0;
			eri.EachPixelConv2ERI(cams[segi], u, v, pixelI, pixelJ);
			if (pixelI < PxU)
				PxU = pixelI;

			v = cams[segi].h - 1;
			eri.EachPixelConv2ERI(cams[segi], u, v, pixelI, pixelJ);
			if (pixelI > PxD)
				PxD = pixelI;

		}
		int half;
		Vec3b insidecolor(255, 0, 0);
		cout << PxL << " " << PxR << " " << PxU << " " << PxD << endl;	

		int left = newERI.cols / 2-(((PxR - PxL))/2);
		int up = newERI.rows / 2-(((PxD - PxU)) / 2);

		Rect RectangleToDraw(left, up, (PxR-PxL), (PxD-PxU));
		rectangle(newERI, RectangleToDraw.tl(), RectangleToDraw.br(), Scalar(0, 0, 255), 2, 8, 0);

				
		/*
		for (int u = PxL; u < PxR; u++)
		{
			for (int v = PxU; v < PxD; v++)
			{
				
				newERI.at<Vec3b>(v, u) = insidecolor;
			}
		}
		*/
		
		namedWindow("sample", WINDOW_NORMAL);
		resizeWindow("sample", 600, 400);
		imshow("sample", newERI);
		waitKey(10);
	}//end of fi loop

}




/*************************************************************************************************************/
/**************************************************************************************************************/
//take a image and created a distorted image from it based on compression factor
//reference Dr. Popescu GPC paper.


Mat Path::EncodeNew(PPC camera1, int compressionfactor, Mat frame, vector<float>& We1, vector<float>& Het1, vector<float>& Heb1, vector<float>& R0x1, vector<float>& R0y1, vector<float>& R0R1_1, vector<float>& R0R4_1)
{
	if (frame.empty())
	{
		cout << "Error loading image" << endl;

	}

	/********************Get middle point and create create bounding box*******************/
	/*****************Ret is used to check weather the image has right before left (In case of image splited between end and start of the ERI***********************/
	float hfov = 110.0f;
	PPC camera(hfov*1.2, 1168 * 1.2, 657 * 1.2);  //size property should be exactly same with camera1.
	// same as camera1
	V3 pb = camera.GetVD();	
	
	V3 pa = camera1.GetVD();
	
	// build local coordinate system of RERI
	V3 xaxis = camera1.a.UnitVector();
	V3 yaxis = camera1.b.UnitVector()*-1.0f;
	V3 zaxis = xaxis ^ yaxis;
	M33 reriCS;
	reriCS[0] = xaxis;
	reriCS[1] = yaxis;
	reriCS[2] = zaxis;

	Mat midcorrectedmat(frame.rows, frame.cols, frame.type());
	RotateXYaxisERI2RERI(frame, midcorrectedmat, pb, pa, reriCS);


	float pxl; float pxr; float pxu; float pxd;	
	DrawBoundinigBoxframe(midcorrectedmat, camera, pxl, pxr, pxu, pxd);
	
	float PxL = pxl;
	float PxR = pxr;
	float PxU = pxu;
	float PxD = pxd;

	
	//cout << PxL <<" "<<PxR<<" "<<PxU<<" "<<PxD<< endl;

	//system("pause");


	float midPx = (PxR + PxL) / 2;
	float boundingboxlength = PxR - PxL;


	float R0x = ((float)frame.cols / 2.0f - (float)boundingboxlength / 2.0f);
	float R0y = ((float)frame.rows / 2.0f - (float)(PxD - PxU) / 2.0f);;
	int Q0x = 0;
	int Q0y = frame.rows;
	int R0R1 = PxD - PxU;
	int R0R4 = boundingboxlength;
	float We = (float)(frame.cols - boundingboxlength) / (float)(2 * compressionfactor);
	float Het = (float)R0y / (float)compressionfactor;
	float Heb = (float)(frame.rows - (R0y + R0R1)) / (float)compressionfactor;


	//print(R0R1 << " " << R0R4 << " " << R0x<<" " << R0y<<" " << We<<" " << Het<<" " << Heb);

	Mat distortedframemat = Mat::zeros((Het + R0R1 + Heb), (2 * We + R0R4), frame.type());
	Mat tmp = midcorrectedmat(Rect(R0x, R0y, R0R4, R0R1));  //midcorrected one is CRERI

	/*********************** create a red outline********************************************/
	Rect RectangleToDraw(0, 0, tmp.cols, tmp.rows);
	rectangle(tmp, RectangleToDraw.tl(), RectangleToDraw.br(), Scalar(0, 0, 255), 2, 8, 0);

	tmp.copyTo(distortedframemat(Rect(We, Het, R0R4, R0R1)));     //copy the bounding box and paste in the distortedframemat image. 

	/*


		Q(0,0)
			___________________________________________________________________________________________________
			|\												/ \													|
			|	\	C										 |(R0y)												|
			|		\		P()								 |													|
			|		:	\	 ________________________________|_______________________________					|
			|		:		|\								 |	   / \						|					|
			|		:		|	\	M						 |		|Het					|					|
			|		:		|		\						 |		|						|					|
			|		:		|		.	\	R0(R0x,R0y)		 |		|	   R4(R0x+R4R1,R0y)    					|					|
			|		:    	|		.		\_______________\_/____\_/_____										|					|
			|	<------(R0x)-------------->	|								|				|					|
			|		:		|		.		|								|				|					|
			|		:		|<........We..>	|								|				|					|
			|		:       |		.	    |								|				|					|
			D(Dx,Dy):<------cfactor*d--->/	|								|				|					|
			|		:		|		./		|								|				|					|
			|		:		|	/	.<--d-->(R0x, Roy+R0R1)	     			|				|					|
			|		:	  /	|		.	 R1 |______________________________	|									|					|
			|		:/		|		.		/												|					|
			|		:		|		.	/													|					|
			|		:		|		/														|					|
			|		:		|	/	N														|					|
			|		:		|/______________________________________________________________|					|
			|		:																							|
			|		:	/																						|
			|		/E																							|
			|	/																								|
			|/__________________________________________________________________________________________________|



	Q: original midcorreccted ERI image
	P: distorted (compressed) ERI image
	R: bounding box of the visualized ERI pixels
	R0x=horizontal distance between original ERI and bounding box
	R0y= same for vertical axis
	We= thickness of padding in x axis (x distance between P and R)
	Het=thickness of upper region
	Heb=thickness of lower region
	MN=line being transformed
	EC=line which MN will be transfered



	*/

	/*****************Encode each of the four region: left, top, right, bottom********************/
	int mxdrow = distortedframemat.rows - 1;
	int mxdcol = distortedframemat.cols - 1;
	int mxcrow = midcorrectedmat.rows - 1;
	int mxccol = midcorrectedmat.cols - 1;

	//to calculated nonuniform distance and then correct it by rotatiing the array/vector//

	/*
	In this region with increasing the array comes in oppossite direction. means when col increases
	d actually increases in inverse direction.  Thats why the
	array needs to be inversed. For this reason we created next two loop. First one
	calculate inverted d and second one correct it.
	f(d)=axx+bx+c; cond1: f(both_edge_of_dist_ERI)= both_edge_of_big_ERI; f'(d_inner_edge_of_dist)=1
	to make sampling constant with the inner rectangle
	the equation becomes b=1; a=(100-10)/10*10. Where d=0 maps to 0 and d=10 maps to 100:
	means edges are at 0,10 for distmat and 0,100 for originalmat
	*/

	vector <float> nonuniformD;
	vector <float > nonuniformDrowcorrected;
	//to understand logic for this two loop, print them somewhere and see with 
	//increasing col quad_out keep increasing the gap. this d here is not our real d.
	//read d starts from the R0R1 and go towards QQ1. So we need to turn the array top to bottom
	for (int col = 0; col < We; col++)
	{
		float d = col;
		float quad_out = ((float)(R0x - We) / (float)(We*We))*d*d + d;
		nonuniformD.push_back(quad_out);

	}

	for (int i = nonuniformD.size() - 1; i > -1; i--)
	{
		nonuniformDrowcorrected.push_back(nonuniformD[i]);

	}

	//////////////
	for (int col = 0; col < We; col++)
	{
		float Dx = R0x - nonuniformDrowcorrected[col];
		//print("Dx: " << Dx << endl);
		for (int row = 0; row < distortedframemat.rows; row++)
		{
			float xx = (float)col / (float)row;
			float yy = (float)We / (float)Het;
			if (xx < yy && col < (float)(We*(float)(mxdrow - row) / (float)(Heb)))
			{
				//region L this one using the technique of using distance to find a point with a slope m//

				//float Dx = R0x - nonuniformDrowcorrected[col];
				float Cx = Dx;
				float Ex = Dx;
				float Cy = Cx * (float)R0y / (float)R0x;

				//use equation to find E,C, M,N. We know their x distance, put that in line equation to find y distance//

				float My = ((R0x - We) + col) * (float)R0y / ((float)R0x);
				float Ny = mxcrow + (((R0x - We) + col) * (float)(R0y + R0R1 - mxcrow)) / (float)R0x;
				float Ey = mxcrow + (Ex * (float)(R0y + R0R1 - mxcrow)) / (float)R0x;
				float Dy = Cy + (float)(Ey - Cy)*(float)(row + R0y - Het - My) / (float)(Ny - My);
				distortedframemat.at<Vec3b>(row, col) = midcorrectedmat.at<Vec3b>(Dy, Dx);

				bilinearinterpolation(distortedframemat, midcorrectedmat, row, col, Dy, Dx);
			}
		}
	}
	//print("loop1" << endl);

	//same logic for quadratic equation of region one

	vector <float> nonuniformD2;
	vector <float > nonuniformDrowcorrected2;

	for (int row = 0; row < Het; row++)
	{
		float d = row;
		float quad_out = ((float)(R0y - Het) / (float)(Het*Het))*d*d + d;
		nonuniformD2.push_back(quad_out);

	}

	for (int i = nonuniformD2.size() - 1; i > -1; i--)
	{
		nonuniformDrowcorrected2.push_back(nonuniformD2[i]);
		//cout << "i:" << i << " d: " << nonuniformD[i] << endl;
	}



	for (int row = 0; row < Het; row++)
	{
		for (int col = 0; col < distortedframemat.cols; col++)
		{
			float xx = (float)col / (float)row;
			float yy = (float)We / (float)Het;
			if (xx > yy && row < (float)(Het*(float)(mxdcol - col) / (float)(We)))
			{
				//Next two commented line for uniform sampling
				//float d = Het-row;  //get distance between current line with the base line			
				//float Dy = R0y - compressionfactor * d;  //Get Dy by multiplying d with factor, this line in orig ERI represent line d of distorted ERI

				float Dy = R0y - nonuniformDrowcorrected2[row];

				float Cx = Dy * (float)R0x / ((float)R0y);  //Cy==Dy

				//use equation to find E,C, M,N. We know their x distance, put that in line equation to find y distance//

				float Mx = ((R0y - Het) + row) * (float)R0x / ((float)R0y);
				float Nx = mxccol + ((R0y - Het) + row) * (float)(R0x + R0R4 - mxccol) / (float)R0y;
				float Ex = mxccol + Dy * (float)(R0x + R0R4 - mxccol) / (float)R0y;
				float Dx = Cx + (float)(Ex - Cx)*(float)(col + R0x - We - Mx) / (float)(Nx - Mx);
				distortedframemat.at<Vec3b>(row, col) = midcorrectedmat.at<Vec3b>(Dy, Dx);
				//print(Dy << ","<<Dx << endl);
				bilinearinterpolation(distortedframemat, midcorrectedmat, row, col, Dy, Dx);
			}
		}
	}

	//print("loop2" << endl);
	for (int col = We + R0R4; col < distortedframemat.cols; col++)
	{
		float d = col - We - R0R4;
		float quad_out = ((float)(R0x - We) / (float)(We*We))*d*d + d;
		//float Dx = R0x + R0R4 + compressionfactor * d;
		float Dx = R0x + R0R4 + quad_out;

		for (int row = 0; row < distortedframemat.rows; row++)
		{
			float xx = Heb * ((float)(col - mxdcol) / (float)We) + mxdrow;

			if ((row < xx) && (row > (float)(Het*(float)(mxdcol - col) / (float)(We))))
			{
				//3rd region. This region is alligned with our quadratic equation means
				// when col increase d also increase in same direction. So we dont need
				// additional work we did in region 1			



				float Cy = R0y * (float)(mxccol - Dx) / (float)(mxccol - R0x - R0R4);
				float My = R0y * (float)(mxccol - (R0x - We + col)) / (float)(mxccol - R0x - R0R4);
				float Ey = mxcrow + (Dx - mxccol)*(float)(mxcrow - R0y - R0R1) / (float)(mxccol - R0x - R0R4);
				float Ny = mxcrow + ((R0x - We + col) - mxccol)*(float)(mxcrow - R0y - R0R1) / (float)(mxccol - R0x - R0R4);
				float Dy = Cy + (Ey - Cy)*(float)(row + R0y - Het - My) / (float)(Ny - My);
				distortedframemat.at<Vec3b>(row, col) = midcorrectedmat.at<Vec3b>(Dy, Dx);
				bilinearinterpolation(distortedframemat, midcorrectedmat, row, col, Dy, Dx);

			}
		}
		//print("Dx2:" << Dx << endl);
	}

	//print("loop3" << endl);
	for (int row = Het + R0R1; row < distortedframemat.rows; row++)
	{
		for (int col = 0; col < distortedframemat.cols; col++)
		{
			float xx = Heb * ((float)(col - mxdcol) / (float)We) + mxdrow;
			if (row > xx && col > (float)(We*(float)(mxdrow - row) / (float)(Heb)))
			{
				//4th region has same logic as second region for quad//

				float d = row - (Het + R0R1);
				float quad_out = ((float)(midcorrectedmat.rows - R0y - R0R1 - Heb) / (float)(Heb*Heb))*d*d + d; //Should replace Roy
				float Dy = (R0y + R0R1) + quad_out;
				float Cx = R0x * ((float)(Dy - mxcrow) / (float)(R0y + R0R1 - mxcrow));
				float Mx = R0x * ((float)(row + R0y - Het - mxcrow) / (float)(R0y + R0R1 - mxcrow));
				float Nx = mxccol + (mxcrow - (row + R0y - Het))*((float)(mxccol - R0x - R0R4) / (float)(R0y + R0R1 - mxcrow));
				float Ex = mxccol + (mxcrow - Dy)*((float)(mxccol - R0x - R0R4) / (float)(R0y + R0R1 - mxcrow));
				float Dx = Cx + (float)(Ex - Cx)*(float)(col + R0x - We - Mx) / (float)(Nx - Mx);
				//distortedframemat.at<Vec3b>(row, col) = midcorrectedmat.at<Vec3b>(Dy, Dx);
				bilinearinterpolation(distortedframemat, midcorrectedmat, row, col, Dy, Dx);
			}
		}
	}
	//print("loop4" << endl);
	float overallcompressionfactor = 100 * distortedframemat.rows*distortedframemat.cols / (midcorrectedmat.rows*midcorrectedmat.cols);

	//OverlayImage(&midcorrectedmat, &distortedframemat, Point((R0x-We), (R0y-Het)));
	//eri.VisualizeNeededPixels(frame, &cams[segi]);
	We1.push_back(We);
	Het1.push_back(Het);
	Heb1.push_back(Heb);
	R0x1.push_back(R0x);
	R0y1.push_back(R0y);
	R0R1_1.push_back(R0R1);
	R0R4_1.push_back(R0R4);
	//print(We<<","<< R0x<<","<< Het<<" "<< Heb<<" "<< R0y<<endl);
	return distortedframemat;

}//mainloop of End of Encoding





/*************************************************************************************************************/
/**************************************************************************************************************/
//take a image and created a distorted image from it based on compression factor
//reference Dr. Popescu GPC paper.





Mat Path::DecodeNewNonLinV2(Mat encoded_image, float var[10], int compressionfactor, PPC camera1)
{
	float compressionfactorX = compressionfactor;
	float compressionfactorY = (float)compressionfactor / (float)1;


	int ERI_w = var[0];
	int ERI_h = var[1];
	float We = var[2];
	float Het = var[3];
	float Heb = Het;

	float R0R4 = ERI_w - 2 * We*compressionfactorX;
	float R0R1 = ERI_h - 2 * Het*compressionfactorY;
	float R0x = We * compressionfactor;
	float R0y = Het * compressionfactor;
	
	//cout << We << " " << Het << " " << R0R1 << " R0R4" <<R0R4<< endl;


	Mat decodedframe(ERI_h, ERI_w, encoded_image.type());
	Mat tmp = encoded_image(Rect(We, Het, R0R4, R0R1));

	/**********************************Recreate Mapping******************************************/

	vector <float> nonuniformDrev;
	vector <float > nonuniformDrowrev;
	//to understand logic for this two loop, print them somewhere and see with 
	//increasing col quad_out keep increasing the gap. this d here is not our real d.
	//read d starts from the R0R1 and go towards QQ1. So we need to turn the array top to bottom

	
	for (int col = 0; col < R0x; col++)
	{
		float j = (float)col / (float)R0x;
		float a =1- (float)1 / (float)compressionfactorX-0;
		float b = (1 - a);
		a = a*(-1);		
		float quad_out = R0x*(b - (float)sqrt(b*b - 4 * a*j)) / (float)(2 * a*compressionfactorX);
		//cout << a << " " << b << " " << quad_out << endl;
		nonuniformDrev.push_back(quad_out);

	}

	vector <float> nonuniformDrev2;
	vector <float > nonuniformDrowrev2;


	for (int row = 0; row < R0y; row++)
	{
		float j = (float)row / (float)R0y;
		float a =1- ((float)1 / (float)compressionfactorY);
		float b = (1 - a);
		a = a * (-1);
		float quad_out = R0y * (b - (float)sqrt(b*b - 4 * a*j)) / (float)(2 * a*compressionfactorY);
		//cout << a << " " << b << " " << quad_out << endl;
		nonuniformDrev2.push_back(quad_out);

	}




	/*********************** create a red outline********************************************/
	Rect RectangleToDraw(0, 0, tmp.cols, tmp.rows);
	//rectangle(tmp, RectangleToDraw.tl(), RectangleToDraw.br(), Scalar(255, 0, 255), 2, 8, 0);
	tmp.copyTo(decodedframe(Rect(R0x, R0y, R0R4, R0R1)));     //copy the bounding box and paste in the distortedframemat image. 
	int mxrow = decodedframe.rows - 1;
	int mxcol = decodedframe.cols - 1;
	/***************************Region 01: left *****************************************/
	for (int col = 0; col < R0x; col++)
	{
		float d = R0x - col;
		float Dx = R0x-nonuniformDrev[d];
		//float Dx = R0x - d / compressionfactorX;
		//cout << "decode col: "<<col<<" Dx: " << (Dx - (R0x - We)) << endl;
		
		for (int row = 0; row < decodedframe.rows; row++)
		{
			Vec3b insidecolor(255, 0, 0);

			float x1 = col * (float)R0y / (float)R0x;
			float x2 = mxrow - (float)col*(mxrow - (R0y + R0R1)) / (float)R0x;

			if ((row > x1) && (row < x2))
			{// decodedframe.at<Vec3b>(row, col) = insidecolor;
				
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

				if (disty >= encoded_image.rows)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					disty = encoded_image.rows - 1;
				}
				if (distx >= encoded_image.cols)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					distx = encoded_image.cols - 1;
				}
				if (disty < 0)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					disty = 0;
				}
				if (distx < 0)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					distx = 0;
				}
				//cout << distx << " " << disty << endl;
				//decodedframe.at<Vec3b>(row, col) = encoded_image.at<Vec3b>(disty,distx );
				bilinearinterpolation(decodedframe, encoded_image, row, col, disty, distx);
			}
		}
	}  //end region 1

	//print("dloop1" << endl);
	//region 3

	

	for (int col = R0x + R0R4; col < mxcol; col++)
	{
		float d = col - (R0x + R0R4);
		float dx = col;
		float Dx = nonuniformDrev[d] + (R0x + R0R4);;
		//cout << "decode col2: " << col << " Dx: " << (Dx - (R0x - We)-We-R0R4) << endl;
		for (int row = 0; row < mxrow; row++)
		{
			Vec3b insidecolor(255, 0, 0);

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
				if (disty >= encoded_image.rows)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					disty = encoded_image.rows - 1;
				}
				if (distx >= encoded_image.cols)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					distx = encoded_image.cols - 1;
				}
				if (disty < 0)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					disty = 0;
				}
				if (distx < 0)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					distx = 0;
				}
				//print(distx << " " << disty << " " << encoded_image.size() << endl);
				//decodedframe.at<Vec3b>(row, col) = encoded_image.at<Vec3b>(disty, distx);
				bilinearinterpolation(decodedframe, encoded_image, row, col, disty, distx);

			}
		}
	}  //end region 3


	//print("dloop2" << endl);

	//region 2: top

	for (int row = 0; row < R0y; row++)
	{
		float d = R0y - row;		
		float dy = row;
		//cout << d <<" "<<nonuniformDrev2.size()<< endl;
		float Dy = R0y - nonuniformDrev2[d];
		//cout << "decode r2: " << row << " Dy: " << (Dy - (R0y - Het) ) << endl;
		for (int col = 0; col < decodedframe.cols; col++)
		{
			Vec3b insidecolor(255, 0, 0);

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

				if (disty > encoded_image.rows - 1)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					disty = encoded_image.rows - 1;
				}

				if (distx > encoded_image.cols - 1)
				{
					//print(" danger1 " << distx << endl);
					//print(distx << " " << disty << " " << encoded_image.size() << endl);
					distx = encoded_image.cols - 1;
				}
				if (disty < 0)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					disty = 0;
				}
				if (distx < 0)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					distx = 0;
				}

				//decodedframe.at<Vec3b>(row, col) = encoded_image.at<Vec3b>(disty, distx);

				bilinearinterpolation(decodedframe, encoded_image, row, col, disty, distx);
			}
		}
	}  //end region 2

	//print("dloop3" << endl);
	//region 4: bottom

	for (int row = R0y + R0R1; row < decodedframe.rows; row++)
	{
		float d = row - (R0y + R0R1);
		float Dy = nonuniformDrev2[d] + (R0y + R0R1);
		for (int col = 0; col < decodedframe.cols; col++)
		{
			Vec3b insidecolor(255, 0, 0);

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
				if (disty >= encoded_image.rows)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					disty = encoded_image.rows - 1;
				}
				if (distx >= encoded_image.cols)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					distx = encoded_image.cols - 1;
				}
				if (disty < 0)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					disty = 0;
				}
				if (distx < 0)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					distx = 0;
				}
				//decodedframe.at<Vec3b>(row, col) = encoded_image.at<Vec3b>(disty, distx);
				bilinearinterpolation(decodedframe, encoded_image, row, col, disty, distx);
			}
		}
	}  //end region 4
	//print("dloop4" << endl);
	return decodedframe;

}






/*************************************************************************************************************/
/**************************************************************************************************************/
//take a image and created a distorted image from it based on compression factor
//reference Dr. Popescu GPC paper.
Mat Path::EncodeLinear(Mat frame, struct var * var1, PPC camera1, int compressionfactor)
{	
	
	if (frame.empty())
	{
		cout << "Error loading image"<< endl;
		
	}

	/********************Get middle point and create create bounding box*******************/
	/*****************Ret is used to check weather the image has right before left (In case of image splited between end and start of the ERI***********************/
	float hfov = 110.0f;
	PPC camera(hfov, 1168, 657);  //size property should be exactly same with camera1.
	//same as camera1
	V3 pb = camera.GetVD();

	V3 pa = camera1.GetVD();

	// build local coordinate system of RERI
	V3 xaxis = camera1.a.UnitVector();
	V3 yaxis = camera1.b.UnitVector()*-1.0f;
	V3 zaxis = xaxis ^ yaxis;
	M33 reriCS;
	reriCS[0] = xaxis;
	reriCS[1] = yaxis;
	reriCS[2] = zaxis;

	Mat midcorrectedmat(frame.rows, frame.cols, frame.type());
	RotateXYaxisERI2RERI(frame, midcorrectedmat, pb, pa, reriCS);

	namedWindow("sample", WINDOW_NORMAL);
	resizeWindow("sample", 800, 400);
	imshow("sample", midcorrectedmat);
	img_write("./Image/test_midcorrected.PNG", midcorrectedmat);
	waitKey(1000);

	float pxl; float pxr; float pxu; float pxd;
	DrawBoundinigBoxframe(midcorrectedmat, camera, pxl, pxr, pxu, pxd);

	float PxL = pxl;
	float PxR = pxr;
	float PxU = pxu;
	float PxD = pxd;


	//cout << PxL <<" "<<PxR<<" "<<PxU<<" "<<PxD<< endl;

	//system("pause");
	int R0R1 = PxD - PxU;
	int R0R4 = PxR - PxL;

	float R0x = ((float)frame.cols / 2.0f - (float)R0R4 / 2.0f);
	float R0y = ((float)frame.rows / 2.0f - (float)(R0R1) / 2.0f);
	int Q0x = 0;
	int Q0y = frame.rows;

	float We = (float)(frame.cols - R0R4) / (float)(2 * compressionfactor);
	float Het = (float)(frame.rows - R0R1) / (float)(2 * compressionfactor);
	float Heb = Het;

	//print("frame size:"<<frame.size() << endl);
	//print("ror1: "<<R0R1 << " ror4:" << R0R4 << " rox:" << R0x<<" roy:" << R0y<<" we:" << We<<" het:" << Het<<" heb:" << Heb);

	Mat distortedframemat = Mat::zeros((Het + R0R1 + Heb), (2 * We + R0R4), frame.type());
	Mat tmp = midcorrectedmat(Rect(R0x, R0y, R0R4, R0R1));  //midcorrected one is CRERI


	/*********************** create a red outline********************************************/
	Rect RectangleToDraw(0, 0, tmp.cols, tmp.rows);
	//rectangle(tmp, RectangleToDraw.tl(), RectangleToDraw.br(), Scalar(0, 255, 255), 2, 8, 0);
	
	tmp.copyTo(distortedframemat(Rect(We, Het, R0R4, R0R1)));     //copy the bounding box and paste in the distortedframemat image. 

	/*
	
	
		Q(0,0)
			___________________________________________________________________________________________________
			|\												/ \													|	
			|	\	C										 |(R0y)												|
			|		\		P()								 |													|
			|		:	\	 ________________________________|_______________________________					|
			|		:		|\								 |	   / \						|					|
			|		:		|	\	M						 |		|Het					|					|
			|		:		|		\						 |		|						|					|
			|		:		|		.	\	R0(R0x,R0y)		 |		|	   R4(R0x+R4R1,R0y)    					|					|
			|		:    	|		.		\_______________\_/____\_/_____										|					|
			|	<------(R0x)-------------->	|								|				|					|
			|		:		|		.		|								|				|					|
			|		:		|<........We..>	|								|				|					|
			|		:       |		.	    |								|				|					|
			D(Dx,Dy):<------cfactor*d--->/	|								|				|					|
			|		:		|		./		|								|				|					|
			|		:		|	/	.<--d-->(R0x, Roy+R0R1)	     			|				|					|
			|		:	  /	|		.	 R1 |______________________________	|									|					|
			|		:/		|		.		/												|					|
			|		:		|		.	/													|					|
			|		:		|		/														|					|
			|		:		|	/	N														|					|
			|		:		|/______________________________________________________________|					|
			|		:																							|
			|		:	/																						|
			|		/E																							|
			|	/																								|
			|/__________________________________________________________________________________________________|
			
	
	
	Q: original midcorreccted ERI image
	P: distorted (compressed) ERI image
	R: bounding box of the visualized ERI pixels
	R0x=horizontal distance between original ERI and bounding box
	R0y= same for vertical axis
	We= thickness of padding in x axis (x distance between P and R)
	Het=thickness of upper region
	Heb=thickness of lower region
	MN=line being transformed
	EC=line which MN will be transfered
	
	
	
	*/

/*****************Encode each of the four region: left, top, right, bottom********************/
	int mxdrow = distortedframemat.rows - 1;
	int mxdcol= distortedframemat.cols - 1;
	int mxcrow = midcorrectedmat.rows - 1;
	int mxccol = midcorrectedmat.cols - 1;


	for (int col = 0; col < We; col++)
	{
		for (int row = 0; row < distortedframemat.rows; row++)
		{
			float xx = (float)col / (float)row;
			float yy = (float)We / (float)Het;
			if (xx < yy && col < (float)(We*(float)(mxdrow - row) / (float)(Heb)))
			{
				//region L this one using the technique of using distance to find a point with a slope m//


				float d = We - col;  //get distance between current line with the base line
				float Dx = R0x - compressionfactor * d;  //Get Dy by multiplying d with factor, this line in orig ERI represent line d of distorted ERI
				float Cx = Dx;
				float Ex = Dx;
				float Cy = Cx * (float)R0y / (float)R0x;

				//use equation to find E,C, M,N. We know their x distance, put that in line equation to find y distance//

				float My = ((R0x - We) + col) * (float)R0y / ((float)R0x);
				float Ny = mxcrow + (((R0x - We) + col) * (float)(R0y + R0R1 - mxcrow)) / (float)R0x;
				float Ey = mxcrow + (Ex * (float)(R0y + R0R1 - mxcrow)) / (float)R0x;
				float Dy = Cy + (float)(Ey - Cy)*(float)(row + R0y - Het - My) / (float)(Ny - My);
				distortedframemat.at<Vec3b>(row, col) = midcorrectedmat.at<Vec3b>(Dy, Dx);
				//print(Dy << ","<<Dx << endl);
				//bilinearinterpolation(distortedframemat, midcorrectedmat, row, col, Dy, Dx);
			}
		}
	}
	//print("loop1" << endl);


	for (int row = 0; row < Het; row++)
	{
		for (int col = 0; col < distortedframemat.cols; col++)
		{
			float xx = (float)col / (float)row;
			float yy = (float)We / (float)Het;
			if (xx > yy && row < (float)(Het*(float)(mxdcol - col) / (float)(We)))
			{
				float d = Het - row;  //get distance between current line with the base line
				float Dy = R0y - compressionfactor * d;  //Get Dy by multiplying d with factor, this line in orig ERI represent line d of distorted ERI
				float Cx = Dy * (float)R0x / ((float)R0y);  //Cy==Dy

				//use equation to find E,C, M,N. We know their x distance, put that in line equation to find y distance//

				float Mx = ((R0y - Het) + row) * (float)R0x / ((float)R0y);
				float Nx = mxccol + ((R0y - Het) + row) * (float)(R0x + R0R4 - mxccol) / (float)R0y;
				float Ex = mxccol + Dy * (float)(R0x + R0R4 - mxccol) / (float)R0y;
				float Dx = Cx + (float)(Ex - Cx)*(float)(col + R0x - We - Mx) / (float)(Nx - Mx);
				distortedframemat.at<Vec3b>(row, col) = midcorrectedmat.at<Vec3b>(Dy, Dx);
				//print(Dy << ","<<Dx << endl);
				//bilinearinterpolation(distortedframemat, midcorrectedmat, row, col, Dy, Dx);
			}
		}
	}

	//print("loop2" << endl);
	for (int col = We + R0R4; col < distortedframemat.cols; col++)
	{
		for (int row = 0; row < distortedframemat.rows; row++)
		{
			float xx = Heb * ((float)(col - mxdcol) / (float)We) + mxdrow;

			if ((row < xx) && (row > (float)(Het*(float)(mxdcol - col) / (float)(We))))
			{
				//3rd region

				float d = col - We - R0R4;
				float Dx = R0x + R0R4 + compressionfactor * d;
				float Cy = R0y * (float)(mxccol - Dx) / (float)(mxccol - R0x - R0R4);
				float My = R0y * (float)(mxccol - (R0x - We + col)) / (float)(mxccol - R0x - R0R4);
				float Ey = mxcrow + (Dx - mxccol)*(float)(mxcrow - R0y - R0R1) / (float)(mxccol - R0x - R0R4);
				float Ny = mxcrow + ((R0x - We + col) - mxccol)*(float)(mxcrow - R0y - R0R1) / (float)(mxccol - R0x - R0R4);
				float Dy = Cy + (Ey - Cy)*(float)(row + R0y - Het - My) / (float)(Ny - My);
				distortedframemat.at<Vec3b>(row, col) = midcorrectedmat.at<Vec3b>(Dy, Dx);
				//bilinearinterpolation(distortedframemat, midcorrectedmat, row, col, Dy, Dx);

			}
		}
	}

	//print("loop3" << endl);
	for (int row = Het + R0R1; row < distortedframemat.rows; row++)
	{
		for (int col = 0; col < distortedframemat.cols; col++)
		{
			float xx = Heb * ((float)(col - mxdcol) / (float)We) + mxdrow;
			if (row > xx && col > (float)(We*(float)(mxdrow - row) / (float)(Heb)))
			{
				//4th region				
				float d = row - (Het + R0R1);
				float Dy = (R0y + R0R1) + compressionfactor * d;
				float Cx = R0x * ((float)(Dy - mxcrow) / (float)(R0y + R0R1 - mxcrow));
				float Mx = R0x * ((float)(row + R0y - Het - mxcrow) / (float)(R0y + R0R1 - mxcrow));
				float Nx = mxccol + (mxcrow - (row + R0y - Het))*((float)(mxccol - R0x - R0R4) / (float)(R0y + R0R1 - mxcrow));
				float Ex = mxccol + (mxcrow - Dy)*((float)(mxccol - R0x - R0R4) / (float)(R0y + R0R1 - mxcrow));
				float Dx = Cx + (float)(Ex - Cx)*(float)(col + R0x - We - Mx) / (float)(Nx - Mx);
				distortedframemat.at<Vec3b>(row, col) = midcorrectedmat.at<Vec3b>(Dy, Dx);
				//bilinearinterpolation(distortedframemat, midcorrectedmat, row, col, Dy, Dx);
			}
		}
	}
	//print("loop4" << endl);
	float overallcompressionfactor = 100 * distortedframemat.rows*distortedframemat.cols / (midcorrectedmat.rows*midcorrectedmat.cols);

	//OverlayImage(&midcorrectedmat, &distortedframemat, Point((R0x-We), (R0y-Het)));
	//eri.VisualizeNeededPixels(frame, &cams[segi]);


	(var1)->colN = frame.cols;
	(var1)->rowN = frame.rows;
	(var1)->We = We;
	(var1)->Het = Het;//*/

	//print(We<<","<< R0x<<","<< Het<<" "<< Heb<<" "<< R0y<<endl);
	return distortedframemat;

}//mainloop of End of Encoding








Mat Path::DecodeLinear(Mat encoded_image, float var[10], int compressionfactor, PPC camera1)
{
	float compressionfactorX = compressionfactor;
	float compressionfactorY = (float)compressionfactor / (float)1;


	int ERI_w = var[0];
	int ERI_h = var[1];
	float We = var[2];
	float Het = var[3];
	float Heb = Het;

	float R0R4 = ERI_w - 2 * We*compressionfactorX;
	float R0R1 = ERI_h - 2 * Het*compressionfactorY;
	float R0x = We * compressionfactor;
	float R0y = Het * compressionfactor;


	Mat decodedframe(ERI_h, ERI_w, encoded_image.type());
	Mat tmp = encoded_image(Rect(We, Het, R0R4, R0R1));


	/*********************** create a red outline********************************************/
	Rect RectangleToDraw(0, 0, tmp.cols, tmp.rows);
	//rectangle(tmp, RectangleToDraw.tl(), RectangleToDraw.br(), Scalar(255, 0, 255), 2, 8, 0);
	tmp.copyTo(decodedframe(Rect(R0x, R0y, R0R4, R0R1)));     //copy the bounding box and paste in the distortedframemat image. 
	int mxrow = decodedframe.rows - 1;
	int mxcol = decodedframe.cols - 1;
	/***************************Region 01: left *****************************************/
	for (int col = 0; col < R0x; col++)
	{for (int row = 0; row < decodedframe.rows; row++)
		{
			Vec3b insidecolor(255, 0, 0);
			
			float x1 = col *  (float)R0y/(float)R0x;
			float x2 = mxrow- (float)col*(mxrow - (R0y + R0R1))/(float)R0x;
			
				if ((row > x1) && (row < x2))
				{// decodedframe.at<Vec3b>(row, col) = insidecolor;
					float d = R0x - col;
					float dx = col;
					float dy = row;
					float Dx = R0x -d/compressionfactorX;
					float Cx = Dx;
					float Ex = Dx;
					float My = dx*(float)R0y / (float)R0x;
					float Cy =  Cx*(float)R0y / (float)R0x;
					float Ny = mxrow - (float)dx*(mxrow - R0y - R0R1) / (float)R0x;
					float Ey = mxrow - (float)Ex*(mxrow - R0y - R0R1) / (float)R0x;
					float Dy = Ey - (float)((Ey-Cy)*(Ny-dy)) /(float)(Ny-My);
					float distx = Dx - (R0x - We);
					float disty=Dy - (R0y - Het);
					
					if (disty >= encoded_image.rows)
					{
						//cout << distx << " " << disty << " " << encoded_image.size() << endl;
						disty = encoded_image.rows - 1;
					}
					if (distx >= encoded_image.cols)
					{
						//cout << distx << " " << disty << " " << encoded_image.size() << endl;
						distx = encoded_image.cols - 1;
					}
					if (disty < 0)
					{
						//cout << distx << " " << disty << " " << encoded_image.size() << endl;
						disty = 0;
					}
					if (distx < 0)
					{
						//cout << distx << " " << disty << " " << encoded_image.size() << endl;
						distx =0;
					}
					//cout << distx << " " << disty << endl;
					//decodedframe.at<Vec3b>(row, col) = encoded_image.at<Vec3b>(disty,distx );
					bilinearinterpolation(decodedframe, encoded_image, row, col, disty, distx);
				}
		}
	}  //end region 1

	//print("dloop1" << endl);
	//region 3


	for (int col = R0x+R0R4; col < mxcol; col++)
	{
		for (int row = 0; row < mxrow; row++)
		{
			Vec3b insidecolor(255, 0, 0);

			float y1 = (float)R0y*(mxcol-col) /(float)(mxcol-R0x-R0R4);
			float y2 = mxrow + (float)(col-mxcol)*(mxrow - (R0y + R0R1)) / (float)(mxcol-(R0x+R0R4));

			if ((row > y1) && (row < y2))
			{
				float d = col-(R0x + R0R4);
				float dx = col;
				float dy = row;
				float Dx = d / compressionfactorX+ (R0x + R0R4);;
				float Cx = Dx;
				float Ex = Dx;
				float My = (float)R0y*(mxcol - dx) / (float)(mxcol - R0x - R0R4);
				float Cy = (float)R0y*(mxcol- Dx) / (float)(mxcol - R0x - R0R4);
				float Ny = mxrow + (float)(dx - mxcol)*(mxrow - (R0y + R0R1)) / (float)(mxcol - (R0x + R0R4));
				float Ey = mxrow + (float)(Dx - mxcol)*(mxrow - (R0y + R0R1)) / (float)(mxcol - (R0x + R0R4));
				float Dy = Ey - (float)((Ey - Cy)*(Ny - dy)) / (float)(Ny - My);
				float distx = Dx - (R0x - We);
				float disty = Dy - (R0y - Het);
				if (disty >= encoded_image.rows)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					disty = encoded_image.rows - 1;
				}
				if (distx >= encoded_image.cols)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					distx = encoded_image.cols- 1;
				}
				if (disty < 0)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					disty = 0;
				}
				if (distx < 0)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					distx = 0;
				}
				//print(distx << " " << disty << " " << encoded_image.size() << endl);
				//decodedframe.at<Vec3b>(row, col) = encoded_image.at<Vec3b>(disty, distx);
				bilinearinterpolation(decodedframe, encoded_image, row, col, disty, distx);
				
			}
		}
	}  //end region 3


	//print("dloop2" << endl);

	//region 2: top

	for (int row = 0; row < R0y; row++)
	{
		for (int col = 0; col < decodedframe.cols; col++)
		{
			Vec3b insidecolor(255, 0, 0);

			float y1 = col * (float)R0y / (float)R0x;
			float y2 = (float)R0y*(mxcol - col) / (float)(mxcol - R0x - R0R4);

			if ((row < y1) && (row < y2))
			{
				float d = R0y-row;
				float dx = col;
				float dy = row;
				float Dy = R0y - d / compressionfactorY;
				float Cy = Dy;
				float Ey = Dy;
				float Mx = dy * (float)R0x / (float)R0y;
				float Cx = Dy * (float)R0x / (float)R0y;;
				float Nx=  mxcol - dy*(float)(mxcol - R0x - R0R4)/(float)R0y;
				float Ex = mxcol - Dy * (float)(mxcol - R0x - R0R4) / (float)R0y;
				float Dx = Ex - (float)((Ex - Cx)*(Nx - dx)) / (float)(Nx - Mx);
				float distx = Dx- (R0x - We);
				float disty = Dy - (R0y - Het);

				if (disty > encoded_image.rows-1)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					disty = encoded_image.rows - 1;
				}

				if (distx > encoded_image.cols-1)
				{
					//print(" danger1 " << distx << endl);
					//print(distx << " " << disty << " " << encoded_image.size() << endl);
					distx = encoded_image.cols - 1;
				}
				if (disty < 0)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					disty = 0;
				}
				if (distx < 0)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					distx = 0;
				}
				
				//decodedframe.at<Vec3b>(row, col) = encoded_image.at<Vec3b>(disty, distx);
				
				bilinearinterpolation(decodedframe, encoded_image, row, col, disty, distx);
			}
		}
	}  //end region 2

	//print("dloop3" << endl);
	//region 4: bottom
	
	for (int row = R0y+R0R1; row < decodedframe.rows; row++)
	{
		for (int col =0; col < decodedframe.cols; col++)
		{
			Vec3b insidecolor(255, 0, 0);

			float x1=(mxrow-row)*(float)R0x / (mxrow - (R0y + R0R1));
			float x2 =mxcol+(row - mxrow)*(float)(mxcol - (R0x + R0R4))/(mxrow - (R0y + R0R1));

			if ((col > x1) && (col < x2))
			{
				float d = row-(R0y + R0R1);
				float dx = col;
				float dy = row;
				float Dy =(float)d/(float)compressionfactorY+ (R0y + R0R1);
				float Cy = Dy;
				float Ey = Dy;
				float Mx = (mxrow - dy)*(float)R0x / (mxrow - (R0y + R0R1));
				float Cx = (mxrow - Dy)*(float)R0x / (mxrow - (R0y + R0R1));
				float Nx = mxcol + (dy - mxrow)*(float)(mxcol - (R0x + R0R4)) / (mxrow - (R0y + R0R1));
				float Ex = mxcol + (Dy - mxrow)*(float)(mxcol - (R0x + R0R4)) / (mxrow - (R0y + R0R1));
				float Dx = Ex - (float)((Ex - Cx)*(Nx - dx)) / (float)(Nx - Mx);
				float distx = Dx - (R0x - We);
				float disty = Dy - (R0y - Het)-1;
				if (disty >= encoded_image.rows)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					disty = encoded_image.rows - 1;
				}
				if (distx >= encoded_image.cols)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					distx = encoded_image.cols - 1;
				}
				if (disty <0)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					disty =0;
				}
				if (distx <0)
				{
					//cout << distx << " " << disty << " " << encoded_image.size() << endl;
					distx =0;
				}
				//decodedframe.at<Vec3b>(row, col) = encoded_image.at<Vec3b>(disty, distx);
				bilinearinterpolation(decodedframe, encoded_image, row, col, disty, distx);
			}
		}
	}  //end region 4
	//print("dloop4" << endl);
	return decodedframe;

}

/*
Overlay image: Put one image transparantely on top of other image
*/
void Path::OverlayImage(Mat* src, Mat* overlay, const Point& location)
{
	for (int y = max(location.y, 0); y < src->rows; ++y)
	{
		int fY = y - location.y;

		if (fY >= overlay->rows)
			break;

		for (int x = max(location.x, 0); x < src->cols; ++x)
		{
			int fX = x - location.x;

			if (fX >= overlay->cols)
				break;

			double opacity = ((double)overlay->data[fY * overlay->step + fX * overlay->channels() + 3]) / 255;

			for (int c = 0; opacity > 0 && c < src->channels(); ++c)
			{
				unsigned char overlayPx = overlay->data[fY * overlay->step + fX * overlay->channels() + c];
				unsigned char srcPx = src->data[y * src->step + x * src->channels() + c];
				src->data[y * src->step + src->channels() * x + c] = srcPx * (1. - opacity) + overlayPx * opacity;
			}
		}
	}
}// End Overlay image


//video encode function*************************************************************************/

vector<Mat> Path::videoencode(char* fname, int lastFrame, vector<float>& We, vector<float>& Het, vector<float>& Heb, vector<float>& R0x, vector<float>& R0y, vector<float>& R0R1, vector<float>& R0R4, int compressionfactor)
{
	VideoCapture cap(fname);
	if (!cap.isOpened())
	{
		cout << "Cannot open the video file: " << fname << endl;
		waitKey(100000);
		return {};
	}
	
	
	int fps = 30;
	int segi = 0;
	float max_w = 0;
	float max_h = 0;

	vector <Mat> retbuffer;

	for (int fi = 0; fi < lastFrame; fi++)
	{		
		Mat frame;
		cap >> frame;
		if (frame.empty())
		{
			cout << fi << endl;
			cout << "Can not read video frame: " << fname << endl;
			break;
		}
		
		namedWindow("sample", WINDOW_NORMAL);
		resizeWindow("sample", 800, 800);
		imshow("sample", frame);
		waitKey(10000);

		Mat ret;
		segi = GetCamIndex(fi, fps, segi);		
		//ret=EncodeNewNonLinV2(cams[segi], compressionfactor, frame, We, Het, Heb, R0x, R0y, R0R1, R0R4);	
	/*	if (ret.cols > max_w)
		{
			max_w = ret.cols;
		}
		if (ret.rows > max_h)
		{
			max_h = ret.rows;
		}  */
		retbuffer.push_back(ret);		
	}

	cap.release();
	
	VideoWriter writer;
	int codec = VideoWriter::fourcc('H', '2', '6', '4');
	writer.set(VIDEOWRITER_PROP_QUALITY, 30);
	string filename = "./Video/encodingtest/rollerh264encod.MKV";
	cout << "Writing videofile: " << filename << codec << endl;
	writer.open(filename, codec, fps, Size(retbuffer[0].cols, retbuffer[0].rows), true);

	if (!writer.isOpened())
	{
		cerr << "Could not open the output video file for write\n";
		return {};
	}

	for (int i = 0; i < retbuffer.size(); i++)
	{
		Mat temp1;
		Mat ret;
		ret = retbuffer[i];

		imshow("sample", ret);
		waitKey(1000);
		
		//cout << ret.size()<<"," <<max_w<<","<<max_h<< endl;
		if (max_w>ret.cols)
		{
			
			//temp1 = Mat::zeros(max_h, max_w, ret.type());
			//ret.copyTo(temp1(Rect(0, 0, ret.cols, ret.rows)));
			
		//imshow("sample", ret);
		//waitKey(1000);
			writer.write(temp1);

		}
		else 
		{ 
			writer.write(ret); 
		}
		
	}

	writer.release();
	return retbuffer; 
}

void Path::videodecode(char* fname, int lastFrame, int original_w, int original_h, vector<float>& We, vector<float>& Het, vector<float>& Heb, vector<float>& R0x, vector<float>& R0y, vector<float>& R0R1, vector<float>& R0R4, int compressionfactor)
{
	VideoCapture cap(fname);
	if (!cap.isOpened()) {
		cout << "Cannot open the video file: " << fname << endl;
		waitKey(100000);
		return;

	}
	Path path1;
	Mat retdecode;

	namedWindow("sample", WINDOW_NORMAL);
	resizeWindow("sample", 800, 800);


	PPC camera1(110.0f, 800, 400);
	camera1.Tilt(-25);
	VideoWriter writer;
	int codec = VideoWriter::fourcc('H', '2', '6', '4');
	writer.set(VIDEOWRITER_PROP_QUALITY, 80);
	int fps = 30;
	string filename = "./Video/encodingtest/rollerh264decod.avi";
	writer.open(filename, codec, fps, Size(camera1.w,camera1.h), true);

	cout << "Writing videofile: " << filename << codec << endl;

	for (int fi = 0; fi < lastFrame; fi++)
	{
		Mat frame;
		cap >> frame;
		cout << fi <<endl;
		if (frame.empty())
		{
			cout << "Can not read video frame: " << fname << endl;
			waitKey(100000);
			return;
		}

		imshow("sample", frame);
		waitKey(30);

		float var[10];
		var[0] = frame.cols;
		var[1] = frame.rows;
		var[2] = We[fi];
		var[3] = Het[fi];//*/
		
		//need work with this one. Change the code to accomodate 0 angle diff.
		print("comehere" << endl);
		//ret1 = path1.CRERI2Conv(encodframe, var, cf, camera2, heatmap, &svar);


		//retdecode = path1.CRERI2Conv(frame,original_w, original_h, We[fi], Het[fi], Heb[fi], R0x[fi], R0y[fi], R0R1[fi], R0R4[fi], compressionfactor);
			
		
		
		writer.write(retdecode);
	
	}
	writer.release();
}



void Path:: PixelXMapERI2RERI(int ur, int uc, int w, float &u)
{
    u = ur + (uc - w / 2);
	u = (u < 0) ? (w + u) : u; // wrap around ERI if needed (left)
	u = (u > w) ? (u - w) : u; // wrap around ERI if needed (right)
}

void Path::RotateXaxisERI2RERI(Mat originERI, Mat& newERI, PPC camera1)
{
	ERI oERI(originERI.cols, originERI.rows);
	V3 p = camera1.GetVD();
	int uc = oERI.Lon2PixJ(oERI.GetXYZ2Longitude(p));
	float u = 0;
	for (int j = 0; j < newERI.rows; j++)
	{
		for (int i = 0; i < newERI.cols; i++)
		{
			PixelXMapERI2RERI(i, uc, newERI.cols, u);
			newERI.at<Vec3b>(j, i) = originERI.at<Vec3b>(j, u);
		}
	}
	
}



void Path::BuildERI2RERI(Mat originERI,  PPC camera1)
{
	Mat newERI = Mat::zeros(originERI.rows, originERI.cols, originERI.type());	

	
	namedWindow("sample", WINDOW_NORMAL);
	resizeWindow("sample", 800, 400);
	
	// build local coordinate system of RERI
	V3 xaxis = camera1.a.UnitVector();
	V3 yaxis = camera1.b.UnitVector()*-1.0f;
	V3 zaxis = xaxis ^ yaxis;
	M33 reriCS;
	reriCS[0] = xaxis;
	reriCS[1] = yaxis;
	reriCS[2] = zaxis;


	V3 p = camera1.GetVD();
	camera1.Pan(0);
	camera1.Tilt(45.0f);
	V3 p1 = camera1.GetVD();
	RotateXYaxisERI2RERI(originERI, newERI, p, p1, reriCS);

	imshow("sample", newERI);
	waitKey(100000);


}


void Path::BuildERI2RERIVideo(Mat originERI, PPC ReferenceCamera)
{

	VideoCapture cap("./Video/roller.MKV");
	if (!cap.isOpened())
	{
		cout << "Cannot open the video file: "<< endl;
		waitKey(100000);
		return;
	}


	int fps = 30;
	int segi = 0;
	float max_w = 0;
	float max_h = 0;

	
	for (int fi = 0; fi < 100; fi++)
	{
		Mat frame;
		cap >> frame;
		if (frame.empty())
		{
			cout << fi << endl;
			cout << "Can not read video frame: " << endl;
			break;
		}

		Mat newERI = Mat::zeros(frame.rows, frame.cols, frame.type());

		segi = GetCamIndex(fi, fps, segi);

		V3 p = ReferenceCamera.GetVD();
		V3 p1 = cams[segi].GetVD();

		// build local coordinate system of RERI
		V3 xaxis = cams[segi].a.UnitVector();
		V3 yaxis = cams[segi].b.UnitVector()*-1.0f;
		V3 zaxis = xaxis ^ yaxis;
		M33 reriCS;
		reriCS[0] = xaxis;
		reriCS[1] = yaxis;
		reriCS[2] = zaxis;


		RotateXYaxisERI2RERI(frame, newERI, p, p1, reriCS);
		/// extra part from border draw

		ERI eri(cap.get(CAP_PROP_FRAME_WIDTH), cap.get(CAP_PROP_FRAME_HEIGHT));
		int pixelI, pixelJ = 0;
		int PxL = eri.w;
		int PxR = 0;
		int PxU = eri.h;
		int PxD = 0;


		cams[segi] = ReferenceCamera;
		for (int v = 0; v < cams[segi].h; v++)
		{
			int u = 0;
			eri.EachPixelConv2ERI(cams[segi], u, v, pixelI, pixelJ);
			if (pixelJ < PxL)
				PxL = pixelJ;

			u = cams[segi].w - 1;
			eri.EachPixelConv2ERI(cams[segi], u, v, pixelI, pixelJ);
			if (pixelJ > PxR)
				PxR = pixelJ;

		}
		for (int u = 0; u < cams[segi].w; u++)
		{
			int v = 0;
			eri.EachPixelConv2ERI(cams[segi], u, v, pixelI, pixelJ);
			if (pixelI < PxU)
				PxU = pixelI;

			v = cams[segi].h - 1;
			eri.EachPixelConv2ERI(cams[segi], u, v, pixelI, pixelJ);
			if (pixelI > PxD)
				PxD = pixelI;

		}
		int half;
		Vec3b insidecolor(255, 0, 0);
		cout << PxL << " " << PxR << " " << PxU << " " << PxD << endl;

		int left = newERI.cols / 2 - (((PxR - PxL)) / 2);
		int up = newERI.rows / 2 - (((PxD - PxU)) / 2);

		Rect RectangleToDraw(left, up, (PxR - PxL), (PxD - PxU));
		rectangle(newERI, RectangleToDraw.tl(), RectangleToDraw.br(), Scalar(0, 0, 255), 2, 8, 0);


		// finish extra part
		
		namedWindow("sample", WINDOW_NORMAL);
		resizeWindow("sample", 800, 400);

		imshow("sample",newERI);
		waitKey(10);

	}
}


