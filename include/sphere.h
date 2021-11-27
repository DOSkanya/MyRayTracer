#pragma once
#include "myraytracer.h"
#include "hittable.h"
#include "material.h"

class sphere : public hittable{
public:
	sphere() { radius = 0.0; }
	sphere(Point4d p, double r, shared_ptr<material> mat) {
		center = p;
		radius = r;
		mat_ptr = mat;
	}

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
	virtual bool bounding_box(b_box& output_box) const override;

public:
	Point4d center;
	double radius;
	shared_ptr<material> mat_ptr;
};

bool sphere::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
	Vector4d oc = r.orig - center;
	auto a = r.dir.dot(r.dir);
	auto half_b = oc.dot(r.dir);
	auto c = oc.dot(oc) - radius * radius;
	auto discriminant = half_b * half_b - a * c;

	if (discriminant < 0) return false;
	auto sqrtd = sqrt(discriminant);

	auto root = (-half_b - sqrtd) / a;
	if (root < t_min || root > t_max) {
		root = (-half_b + sqrtd) / a;
		if (root < t_min || root > t_max)
			return false;
	}

	rec.t = root;
	rec.p = r.orig + rec.t * r.dir;
	rec.n = (rec.p - center).normalized();
	rec.set_face_normal(r, rec.n);
	rec.mat_ptr = mat_ptr;

	return true;
}

bool sphere::bounding_box(b_box& output_box) const {
	output_box.minimum = Point4d(center.x() - radius, center.y() - radius, center.z() - radius, 1.0);
	output_box.maximum = Point4d(center.x() + radius, center.y() + radius, center.z() + radius, 1.0);
	return true;
}