#pragma once
#include "myraytracer.h"
#include "hittable.h"
#include <vector>

class hittable_list : public hittable {
public:
	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
	virtual bool bounding_box(b_box& output_box) const override;
	void add(shared_ptr<hittable> object) { objects.push_back(object); }
public:
	std::vector<shared_ptr<hittable>> objects;
};

bool hittable_list::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
	hit_record temp;
	bool hit_anything = false;
	auto closest_so_far = t_max;

	for (const auto& item : objects) {
		if (item->hit(r, t_min, closest_so_far, temp)) {
			hit_anything = true;
			closest_so_far = temp.t;//Find the closest point that the ray hit
			rec = temp;
		}
	}
	
	return hit_anything;
}

bool hittable_list::bounding_box(b_box& output_box) const {
	b_box temp;
	objects[0]->bounding_box(output_box);

	for (const auto& object : objects) {
		object->bounding_box(temp);
		output_box = surrounding_box(output_box, temp);
	}
	return true;
}