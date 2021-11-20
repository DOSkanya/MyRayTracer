#pragma once
#include "myraytracer.h"

class ortho {
public:
	ortho() {}
	ortho(Vector4d uu, Vector4d vv, Vector4d ww) {
		u = uu;
		v = vv;
		w = ww;
	}

	void build_from_vector(Vector4d base);
	Vector4d local(const Vector4d& a);
	Vector4d local(double x, double y, double z);
private:
	Vector4d u, v, w;
};

void ortho::build_from_vector(Vector4d base) {
	v = base.normalized();
	Vector4d a = (fabs(v.x()) > 0.9) ? Vector4d(0, 1, 0, 0) : Vector4d(1, 0, 0, 0);
	w = cross(a, v).normalized();
	u = cross(v, w);
}

Vector4d ortho::local(const Vector4d& a) {
	return a.x() * u + a.y() * v + a.z() * w;
}

Vector4d ortho::local(double x, double y, double z) {
	return x * u + y * v + z * w;
}