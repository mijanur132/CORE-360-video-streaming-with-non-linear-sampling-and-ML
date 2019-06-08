#pragma once
#include <iostream>


using namespace std;


class V3 {
public:
	float xyz[3];
	float& operator[](int i);
	float operator*(V3 v1);
	V3 operator-(V3 v1);
	V3 operator+(V3 v1);
	V3 operator*(float scf);
	V3 operator^(V3 v2);
	friend ostream& operator<<(ostream& ostr, V3 v);
	void SetFromColor(unsigned int color);
	unsigned int GetColor();
	float Length();
	V3() {};	
	V3(float x, float y, float z);
	V3 RotateThisPointAboutArbitraryAxis(V3 O, V3 a, float angled);
	V3 RotateThisVectorAboutDirection(V3 a, float angled);
	V3 UnitVector();
	V3 Reflect(V3 r);
	V3 Clamp();
	V3 IntersectRayWithPlane(V3 dir, V3 n, V3 p0);
};