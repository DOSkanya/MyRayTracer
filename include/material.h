#pragma once
#include "myraytracer.h"
#include "ray.h"
#include "texture.h"
#include "hittable.h"
#include "ortho.h"

class material {
public:
	virtual bool scatter(const ray& r_in, const hit_record& hrec, scatter_record& srec) = 0;
	virtual Color3d emitted(const hit_record& hrec) {
		return Color3d(0.0, 0.0, 0.0);
	}
};

class lambertian : public material {
public:
	lambertian(Color3d color) : albedo(make_shared<solid_color>(color)) {}
	lambertian(shared_ptr<solid_color> a) : albedo(a) {}

	virtual bool scatter(const ray& r_in, const hit_record& hrec, scatter_record& srec) {
		srec.scatter_ray.orig = hrec.p;
		ortho coord;
		coord.build_from_vector(hrec.n);
		srec.scatter_ray.dir = hrec.n + random_in_unit_sphere();
		srec.attenuation = albedo->value(hrec.tex.x(), hrec.tex.y());
		return true;
	}
public:
	shared_ptr<texture> albedo; 
};

class diffuse_light : public material {
public:
	diffuse_light() {}
	diffuse_light(shared_ptr<texture> t) : emit(t) {}
	diffuse_light(Color3d c) : emit(make_shared<solid_color>(c)) {}

	virtual bool scatter(const ray& r_in, const hit_record& hrec, scatter_record& srec) {
		return false;
	}

	Color3d emitted(const hit_record& hrec) {
		if (hrec.front_face)
			return emit->value(hrec.tex.x(), hrec.tex.y());
		else
			return Color3d(0, 0, 0);
	}

private:
	shared_ptr<texture> emit;
};