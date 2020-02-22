
#include <math.h>
#include "m33.h"

V3& M33::operator[](int i) {

	return rows[i];

}

ostream& operator<<(ostream& ostr, M33 m) {

	return ostr << m[0] << endl << m[1] << endl << m[2];
}

V3 M33::operator*(V3 v) {

	M33 &m = *this;
	return V3(m[0] * v, m[1] * v, m[2] * v);

}

M33 M33::operator*(M33 m1) {

	M33 &m0 = *this;
	M33 ret;
	ret.SetColumn(0, m0*m1.GetColumn(0));
	ret.SetColumn(1, m0*m1.GetColumn(1));
	ret.SetColumn(2, m0*m1.GetColumn(2));
	return ret;

}

V3 M33::GetColumn(int i) {

	M33 &m = *this;
	return V3(m[0][i], m[1][i], m[2][i]);

}

void M33::SetColumn(int i, V3 c) {

	M33 &m = *this;
	m[0][i] = c[0];
	m[1][i] = c[1];
	m[2][i] = c[2];

}

M33 M33::Inverted() {

	M33 ret;
	V3 a = GetColumn(0), b = GetColumn(1), c = GetColumn(2);
	V3 _a = b ^ c; _a = _a * (1.0f / (a * _a));
	V3 _b = c ^ a; _b = _b * (1.0f / (b * _b));
	V3 _c = a ^ b; _c = _c * (1.0f / (c * _c));
	ret[0] = _a;
	ret[1] = _b;
	ret[2] = _c;

	return ret;

}


void M33::SetRotationAboutY(float angled) {

	float angler = angled * 3.1415f / 180.0f;
	M33 &m = *this;
	m[0] = V3(cosf(angler), 0.0f, sinf(angler));
	m[1] = V3(0.0f, 1.0f, 0.0f);
	m[2] = V3(-sinf(angler), 0.0f, cosf(angler));

}

M33 M33::Transposed() {

	M33 ret;
	ret[0] = GetColumn(0);
	ret[1] = GetColumn(1);
	ret[2] = GetColumn(2);
	return ret;

}
