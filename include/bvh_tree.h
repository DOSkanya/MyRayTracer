#pragma once
#include <vector>
#include <iostream>
#include "myraytracer.h"
#include "hittable.h"
#include "hittable_list.h"
#include "bounding_box.h"

inline bool box_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b, int axis) {
	b_box box_a;
	b_box box_b;

	if (!a->bounding_box(box_a) || !b->bounding_box(box_b))
		std::cerr << "No bounding box in bvh_node constructor.\n";

	return box_a.minimum[axis] < box_b.minimum[axis];
}

bool box_x_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
	return box_compare(a, b, 0);
}

bool box_y_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
	return box_compare(a, b, 1);
}

bool box_z_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
	return box_compare(a, b, 2);
}

class bvh_node : public hittable {
public:
	bvh_node() { left = nullptr; right = nullptr; }
	bvh_node(const hittable_list& scene) : bvh_node(scene.objects, 0, scene.objects.size()) {}
	bvh_node(const std::vector<shared_ptr<hittable>>& src_objects, size_t start, size_t end);

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
	virtual bool bounding_box(b_box& output_box) const override;
public:
	shared_ptr<hittable> left;
	shared_ptr<hittable> right;
	b_box box;
};

bvh_node::bvh_node(const std::vector<shared_ptr<hittable>>& src_objects, size_t start, size_t end) {
	auto objects = src_objects;

	int axis = random_int(0, 2);
	auto comparator = (axis == 0) ? box_x_compare
					: (axis == 1) ? box_y_compare
					: box_z_compare;

	size_t object_span = end - start;

	if (object_span == 1) {
		left = right = objects[start];
	}
	else if (object_span == 2) {
		if (comparator(objects[start], objects[start + 1])) {
			left = objects[start];
			right = objects[start + 1];
		}
		else {
			left = objects[start + 1];
			right = objects[start];
		}
	}
	else {
		std::sort(objects.begin() + start, objects.begin() + end, comparator);

		auto mid = start + object_span / 2;
		left = make_shared<bvh_node>(objects, start, mid);
		right = make_shared<bvh_node>(objects, mid, end);
	}


	b_box box_left, box_right;

	if (!left->bounding_box(box_left)
		|| !right->bounding_box(box_right)) {}

	box = surrounding_box(box_left, box_right);
}

bool bvh_node::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
	if (!box.hit(r, t_min, t_max))
		return false;

	bool hit_left = left->hit(r, t_min, t_max, rec);
	bool hit_right = right->hit(r, t_min, hit_left ? rec.t : t_max, rec);

	return hit_left || hit_right;
}

bool bvh_node::bounding_box(b_box& output_box) const {
	output_box = box;
	return true;
}