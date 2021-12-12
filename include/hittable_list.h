#pragma once
#include "myraytracer.h"
#include "hittable.h"
#include <vector>

class hittable_list : public hittable {
public:
	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
	virtual bool bounding_box(b_box& output_box) const override;
	void add(shared_ptr<hittable> object) { objects.push_back(object); }
	void apply_transformation(double scale, double degree, Vector3d translate);

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

void hittable_list::apply_transformation(double scale, double degree, Vector3d translate) {
	Eigen::Matrix4d scale_matrix, rotate_matrix, translate_matrix, matrix;
	scale_matrix << scale, 0.0, 0.0, 0.0,
		0.0, scale, 0.0, 0.0,
		0.0, 0.0, scale, 0.0,
		0.0, 0.0, 0.0, 1.0;

	double radians = degrees_to_radians(degree);
	rotate_matrix << cos(radians), 0.0, sin(radians), 0.0,
		0.0, 1.0, 0.0, 0.0,
		-sin(radians), 0.0, cos(radians), 0.0,
		0.0, 0.0, 0.0, 1.0;

	translate_matrix << 1.0, 0.0, 0.0, translate.x(),
		0.0, 1.0, 0.0, translate.y(),
		0.0, 0.0, 1.0, translate.z(),
		0.0, 0.0, 0.0, 1.0;
	matrix = translate_matrix * rotate_matrix * scale_matrix;
	for (auto& item : objects) {
		item->apply_vertex_transformation(matrix);
		item->apply_normal_transformation(rotate_matrix);
	}
}