
#include "ppc.h"
#include "m33.h"
#include"image.h"
#include<math.h>
#include<string.h>


/*
#include <C:\opencv\build\include\opencv2\opencv.hpp>
#include <C:\opencv\build\include\opencv2\core\core.hpp>
#include <C:\opencv\build\include\opencv2\highgui\highgui.hpp>


using namespace cv;
*/

PPC::PPC():a(1.0f, 0.0f, 0.0f), b(0.0f, -1.0f, 0.0f),
C(0.0f, 0.0f, 0.0f) {
	float hfov = 130;
	int w = 320;
	int h = 240;
	float hfovr = hfov * 3.14159f / 180.0f;
	float f = -(float)w / (2.0f *tanf(hfovr / 2.0f));
	c = V3(-(float)w / 2.0f, (float)h / 2.0f, f);
	updateInv = 1;
	
}

PPC::~PPC() {
	
}

PPC::PPC(float hfov, int _w, int _h): a(1.0f, 0.0f, 0.0f), b(0.0f, -1.0f, 0.0f),
	C(0.0f, 0.0f, 0.0f), w(_w), h(_h) {

	float hfovr = hfov * 3.14159f / 180.0f;
	float f = -(float)w / (2.0f *tanf(hfovr / 2.0f));
	c = V3(-(float)w / 2.0f, (float)h / 2.0f, f);	
	updateInv = 1;

}	

PPC::PPC(float fov[2], int _w) : a(1.0f, 0.0f, 0.0f), b(0.0f, -1.0f, 0.0f),
C(0.0f, 0.0f, 0.0f), w(_w) {

	float hfov = fov[0];
	float vfov = fov[1];
	float hfovr = hfov * 3.14159f / 180.0f;
	float vfovr = vfov * 3.14159f / 180.0f;
	h = tan(vfovr / 2) / tan(hfovr / 2)*w;	
	float f = -(float)w / (2.0f *tanf(hfovr / 2.0f));
	c = V3(-(float)w / 2.0f, (float)h / 2.0f, f);
	updateInv = 1;	

}

int PPC::Project(V3 p, V3& pp) {
	
	if (updateInv)
	{		
		M33 M;		
		M.SetColumn(0, a);
		M.SetColumn(1, b);
		M.SetColumn(2, c);
		pp[0] = FLT_MAX;
		M_Inv = M.Inverted();
		updateInv=0;

	}
		V3 q =M_Inv *(p - C);
		if (q[2] < 0.0f)
			return 0;
		pp[2] = 1.0f / q[2];
		pp[0] = q[0] / q[2];
		pp[1] = q[1] / q[2];
		return 1;

}

V3 PPC::GetVD() {

	return (a ^ b).UnitVector();

}

float PPC::GetFocalLength() {

	return GetVD()*c;

}

void PPC::ChangeFocalLength(float scf) {

	c = c + GetVD()*(GetFocalLength()*(scf - 1.0f));
	updateInv = 1;

}


void PPC::PositionAndOrient(V3 C1, V3 L1, V3 vpv) {

	V3 vd1 = (L1 - C1).UnitVector();
	V3 a1 = (vd1 ^ vpv).UnitVector() * a.Length();
	V3 b1 = (vd1 ^ a1).UnitVector() * b.Length();
	V3 c1 = vd1 * GetFocalLength() - b1 * ((float)h / 2.0f) - a1 * ((float)w / 2.0f);

	C = C1;
	a = a1;
	b = b1;
	c = c1;
	updateInv = 1;

}


void PPC::Pan(float angled) {


	V3 dv = (b * -1.0f).UnitVector();
	a = a.RotateThisVectorAboutDirection(dv, angled);
	c = c.RotateThisVectorAboutDirection(dv, angled);
	updateInv = 1;

}




void PPC::Tilt(float angled) {


	V3 dv = a.UnitVector();
	b = b.RotateThisVectorAboutDirection(dv, angled);
	c = c.RotateThisVectorAboutDirection(dv, angled);
	updateInv = 1;

}

void PPC::Roll(float angled) {


	V3 dv = GetVD().UnitVector();
	a = a.RotateThisVectorAboutDirection(dv, angled);
	b = b.RotateThisVectorAboutDirection(dv, angled);
	c = c.RotateThisVectorAboutDirection(dv, angled);
	updateInv = 1;

}


#if 0
void PPC::Visualize(PPC *ppc3, FrameBuffer *fb3, float vf, V3 colv) {

	fb3->Draw3DPoint(C, colv, ppc3, 5);

	float scf = vf / GetFocalLength();
	V3 vd = GetVD().UnitVector();
//	fb3->Draw3DSegment(C, colv, C + vd * vf * 3, colv, ppc3);


	fb3->Draw3DSegment(C + c * scf, colv, C + (c + a * (float)w)*scf, colv, ppc3);
	fb3->Draw3DSegment(C, colv, C + (c + a * (float)w)*scf, colv, ppc3);

	fb3->Draw3DSegment(C + (c + a * (float)w)*scf, colv,
		C + (c + a * (float)w + b * (float)h)*scf, colv, ppc3);
	fb3->Draw3DSegment(C, colv,
		C + (c + a * (float)w + b * (float)h)*scf, colv, ppc3);

	fb3->Draw3DSegment(
		C + (c + a * (float)w + b * (float)h)*scf, colv,
		C + (c +  b * (float)h)*scf, colv,
		ppc3);
	fb3->Draw3DSegment(
		C, colv,
		C + (c + b * (float)h)*scf, colv,
		ppc3);

	fb3->Draw3DSegment(
		C + (c + b * (float)h)*scf, colv,
		C+c*scf, colv,
		ppc3);
	fb3->Draw3DSegment(C, colv, C + c * scf, colv, ppc3);

}
#endif

V3 PPC::UnprojectPixel(float uf, float vf, float currf) {

	return C + (a*uf + b * vf + c)*currf * (1.0f /  GetFocalLength());

}


V3 PPC::Unproject(V3 pP) {

	return C + (a*pP[0] + b * pP[1] + c) * (1.0f / pP[2]);

}


#if 0
void PPC::SetIntrinsicsHW() {

	glViewport(0, 0, w, h);

	float zNear = 1.0f;
	float zFar = 1000.0f;
	float scf = zNear / GetFocalLength();
	float left = -a.Length()*(float)w / 2.0f*scf;
	float right = a.Length()*(float)w / 2.0f*scf;
	float top = b.Length()*(float)h / 2.0f*scf;
	float bottom = -b.Length()*(float)h / 2.0f*scf;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(left, right, bottom, top, zNear, zFar);

	glMatrixMode(GL_MODELVIEW);

}
void PPC::SetExtrinsicsHW() {

	V3 L = C + GetVD()*100.0f;
	glLoadIdentity();
	gluLookAt(C[0], C[1], C[2], L[0], L[1], L[2], -b[0], -b[1], -b[2]);

}

#endif

void PPC::SetInterpolated(PPC *ppc0, PPC *ppc1, int stepi, int stepsN) {


	(*this) = *ppc0;

	float f = (float)stepi / (float)(stepsN - 1);
	V3 Ci = ppc0->C + (ppc1->C - ppc0->C) * f;

	V3 L0 = ppc0->C + ppc0->GetVD()*100.0f;
	V3 L1 = ppc1->C + ppc1->GetVD()*100.0f;
	V3 Li = L0 + (L1-L0) * f;

	V3 vpv0 = ppc0->b*-1.0f;
	V3 vpv1 = ppc1->b*-1.0f;
	V3 vpvi = vpv0 + (vpv1 - vpv0)*f;

	PositionAndOrient(Ci, Li, vpvi);
	updateInv = 1;

}


void PPC::RotateAboutAxisThroughEye(V3 v, float theta) {

	a = a.RotateThisVectorAboutDirection(v, theta);
	b = b.RotateThisVectorAboutDirection(v, theta);
	c = c.RotateThisVectorAboutDirection(v, theta);
	updateInv = 1;
	
}


V3 PPC::GetUnitRay(float uf, float vf) {

	V3 ray = a * uf + b * vf + c;
	return ray.UnitVector();

}



