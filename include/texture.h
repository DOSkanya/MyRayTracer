#pragma once
#include "myraytracer.h"

class texture {
public:
	virtual Color3d value(double u, double v) = 0;
};

class solid_color : public texture {
public:
	solid_color(Color3d c) : color(c) {}
	virtual Color3d value(double u, double v) {
		return color;
	}
private:
	Color3d color;
};