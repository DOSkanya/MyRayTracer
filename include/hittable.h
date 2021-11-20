#pragma once
#include "myraytracer.h"
#include "ray.h"
#include "bounding_box.h"

class material;

struct hit_record {
	Point4d p;
	Vector4d n;
	Texture2d tex;
	shared_ptr<material> mat_ptr;
	double t;
	bool front_face;

	inline void set_face_normal(const ray& r, const Vector4d& outward_normal) {
		front_face = r.dir.dot(outward_normal) < 0;
		n = front_face ? outward_normal : -outward_normal;
	}
};

struct scatter_record {
	ray scatter_ray;
	Color3d attenuation;
};

class hittable {
public:
	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const = 0;
	virtual bool bounding_box(b_box& output_box) const = 0;
};