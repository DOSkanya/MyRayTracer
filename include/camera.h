#pragma once
#include "myraytracer.h"
#include "ray.h"

class camera {
public:
	camera(Point4d lookfrom, Point4d lookat, Vector4d vup, 
		double vfov, double aspect_ratio, double focus_dist) {
		auto theta = degrees_to_radians(vfov);
		auto h = tan(theta / 2.0);
		auto viewport_height = focus_dist * 2.0 * h;
		auto viewport_width = aspect_ratio * viewport_height;

		w = (lookat - lookfrom).normalized();
		u = cross(vup, w).normalized();
		v = cross(w, u);

		viewpoint = lookfrom;
		vertical = viewport_height * v;
		horizontal = viewport_width * u;
		lower_left_corner = viewpoint - vertical / 2.0 - horizontal / 2.0 + focus_dist * w;
	}

	ray ray_generate(double s, double t) const {
		return ray(viewpoint, lower_left_corner + s * horizontal + t * vertical - viewpoint);
	}

	Point4d viewpoint;
	Vector4d u, v, w;
	Point4d lower_left_corner;
	Vector4d horizontal, vertical;
};