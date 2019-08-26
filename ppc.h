#pragma once
#include<math.h>
#include "v3.h"
/*
#include <C:\opencv\build\include\opencv2\opencv.hpp>
#include <C:\opencv\build\include\opencv2\core\core.hpp>
#include <C:\opencv\build\include\opencv2\highgui\highgui.hpp>

*/
//using namespace cv;

//class FrameBuffer;

class PPC {
public:
	V3 a, b, c, C; // eye is 3D point C
	int w, h; // image resolution
	int updateInv; //this needs to be set 1 everytime the camera changes

  // horizontal field of view and image resolution
  // this constructor places the eye at (0, 0, 0), view direction is (0, 0, -1), and the up vector is (0, 1, 0)
	PPC(float hfov, int _w, int _h);
	PPC(float fov[2], int _w);
	PPC();
	~PPC();
  // takes a 3D point and projects it on the image plane
  // return 0 if the point is behind the head
  // the image pixel coordinates of the projected point are in pp[0] (pixel column) and pp[1] (pixel row)
  int Project(V3 p, V3 &pp);
	
  V3 GetVD();
	float GetFocalLength();
	void ChangeFocalLength(float scf);
	void PositionAndOrient(V3 C1, V3 L1, V3 vpv);
	void SetInterpolated(PPC *ppc0, PPC *ppc1, int stepi, int stepsN);// takes two cameras and it ask to path and total number of step, create an intermidiete camera.. stepsN 100, stepi=0 ppc0 step100=ppc1 step50=50 middle of both camera
	void Pan(float angled);
	void Tilt(float angled);//+ve tilt dile camera upore uthe, means lat kome
	void Roll(float angled);
//	void Visualize(PPC *ppc3, FrameBuffer *fb3, float vf, V3 colv);
	V3 UnprojectPixel(float uf, float vf, float currf);
	V3 Unproject(V3 pP);
//	void SetIntrinsicsHW();
//	void SetExtrinsicsHW();

	//int upload_image(string path, Mat &image);  //palash

	void RotateAboutAxisThroughEye(V3 v, float theta);
	V3 GetUnitRay(float uf, float vf);

};