#pragma once
#include "myraytracer.h"
#include "ortho.h"

class pdf {
public:
	virtual ~pdf() {}
	virtual double value(const Vector4d& direction) const = 0;
	virtual Vector4d generate() const = 0;
};

class uniform_pdf : public pdf {
public:
	uniform_pdf(const Vector4d& normal) {
		uvw.build_from_vector(normal);
	}
	virtual double value(const Vector4d& direction) const;
	virtual Vector4d generate() const override;
public:
	ortho uvw;
};

double uniform_pdf::value(const Vector4d& direction) const {
	return 1 / (2 * pi);
}

Vector4d uniform_pdf::generate() const {
	//PBR的采样理论是基于经典球面坐标系,xy轴位于平面上，而z轴决定上下
	//渲染器里定义的xyz轴和几何中常用的坐标系不一样，要注意应用场景
	auto xi_1 = random_double();
	auto xi_2 = random_double();
	auto sqrt_value = std::sqrt(1 - xi_1 * xi_1);

	auto x = cos(2 * pi * xi_2) * sqrt_value;
	auto y = xi_1;
	auto z = sin(2 * pi * xi_2) * sqrt_value;
	return uvw.local(x, y, z);
}