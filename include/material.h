#pragma once
#include "myraytracer.h"
#include "ray.h"
#include "texture.h"
#include "hittable.h"
#include "ortho.h"
#include "pdf.h"

class material {
public:
	virtual bool scatter(const ray& r_in, const hit_record& hrec, scatter_record& srec) const = 0;
	virtual Color3d emitted(const hit_record& hrec) const = 0;
	virtual double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const {
		return 0;
	}
};

class lambertian : public material {
public:
	lambertian(Color3d color) : albedo(make_shared<solid_color>(color)) {}
	lambertian(shared_ptr<solid_color> a) : albedo(a) {}

	virtual bool scatter(const ray& r_in, const hit_record& hrec, scatter_record& srec) const {
		//srec.pdf_ptr = make_shared<uniform_pdf>(hrec.n);
		srec.pdf_ptr = make_shared<cosine_pdf>(hrec.n);
		srec.attenuation = albedo->value(hrec.tex.x(), hrec.tex.y());
		return true;
	}
	virtual Color3d emitted(const hit_record& hrec) const {
		return Color3d(0.0, 0.0, 0.0);
	}
	virtual double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const {
		return 1.0 / (2 * pi);
	}
public:
	shared_ptr<texture> albedo; 
};

class diffuse_light : public material {
public:
	diffuse_light() {}
	diffuse_light(shared_ptr<texture> t) : emit(t) {}
	diffuse_light(Color3d c) : emit(make_shared<solid_color>(c)) {}

	virtual bool scatter(const ray& r_in, const hit_record& hrec, scatter_record& srec) const {
		return false;
	}

	virtual Color3d emitted(const hit_record& hrec) const {
		//if (hrec.front_face)
		return emit->value(hrec.tex.x(), hrec.tex.y());
		//else
			//return Color3d(0.0, 0.0, 0.0);
	}

private:
	shared_ptr<texture> emit;
};