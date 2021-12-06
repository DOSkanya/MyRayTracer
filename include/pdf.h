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
 
class cosine_pdf : public pdf {
public:
	cosine_pdf(const Vector4d& normal) {
		uvw.build_from_vector(normal);
	}
	virtual double value(const Vector4d& direction) const override;
	virtual Vector4d generate() const override;
public:
	ortho uvw;
};

double cosine_pdf::value(const Vector4d& direction) const {
	auto cosine = uvw.vv().dot(direction.normalized());
	return cosine <= 0 ? 0 : cosine / pi;
}

Vector4d cosine_pdf::generate() const {
	//改良版的cosine采样
	auto xi_1 = random_double();
	auto xi_2 = random_double();

	auto x = cos(2 * pi * xi_2) * sqrt(xi_1);
	auto y = sqrt(1 - xi_1);
	auto z = sin(2 * pi * xi_2) * sqrt(xi_1);
	return uvw.local(x, y, z);
}

class hittable_pdf : public pdf {
public:
	hittable_pdf(const Point4d& origin, shared_ptr<hittable> object) : o(origin), ptr(object) {}
	virtual double value(const Vector4d& direction) const {
		return ptr->pdf_value(o, direction);
	}
	virtual Vector4d generate() const {
		return ptr->generate_direction(o);
	}

public:
	Point4d o;
	shared_ptr<hittable> ptr;
};

class mixture_pdf : public pdf {
public:
	mixture_pdf(shared_ptr<pdf> p0, shared_ptr<pdf> p1, shared_ptr<pdf> p2) {
		p[0] = p0;
		p[1] = p1;
		p[2] = p2;
	}
	virtual double value(const Vector4d& direction) const {
		return 0.5 * p[0]->value(direction) + 0.25 * p[1]->value(direction) + 0.25 * p[2]->value(direction);
	}
	virtual Vector4d generate() const {
		auto random = random_double();
		if (random < 0.5) return p[0]->generate();
		else if (random >= 0.5 && random < 0.75) return p[1]->generate();
		else return p[2]->generate();
	}
public:
	shared_ptr<pdf> p[3];
};