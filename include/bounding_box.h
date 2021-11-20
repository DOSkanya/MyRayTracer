#pragma once
#include "hittable.h"

class b_box {
public:
	b_box() {}
	b_box(const Point4d& min, const Point4d& max) { minimum = min; maximum = max; }

	Point4d min() { return minimum; }
	Point4d max() { return maximum; }

	bool hit(const ray& r, double t_min, double t_max) const;
public:
	Point4d minimum;
	Point4d maximum;
};

inline bool b_box::hit(const ray& r, double t_min, double t_max) const {
	for (int a = 0; a < 3; a++) {
		auto invD = 1.0f / r.dir[a];
		auto t0 = (minimum[a] - r.orig[a]) * invD;
		auto t1 = (maximum[a] - r.orig[a]) * invD;
		if (invD < 0.0f)
			std::swap(t0, t1);
		t_min = t0 > t_min ? t0 : t_min;
		t_max = t1 < t_max ? t1 : t_max;
		if (t_max <= t_min)
			return false;
	}
	return true;
}

b_box surrounding_box(b_box& box0, b_box& box1) {
	Point4d small(fmin(box0.minimum.x(), box1.minimum.x()),
		fmin(box0.minimum.y(), box1.minimum.y()),
		fmin(box0.minimum.z(), box1.minimum.z()));
	Point4d big(fmax(box0.maximum.x(), box1.maximum.x()),
		fmax(box0.maximum.y(), box1.maximum.y()),
		fmax(box0.maximum.z(), box1.maximum.z()));

	return b_box(small, big);
}