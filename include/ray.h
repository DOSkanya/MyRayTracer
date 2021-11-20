#pragma once
#include "myraytracer.h"

class ray {
public:
	ray() {}
	ray(Point4d o, Vector4d d) {
		orig = o;
		dir = d;
	}

	Point4d orig;
	Vector4d dir;
};