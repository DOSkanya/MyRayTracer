#pragma once

#include "myraytracer.h"
#include "bvh_tree.h"
#include "camera.h"
#include "material.h"
#include <thread>
#include <mutex>

extern std::mutex change;

Color3d ray_color(const ray& r, const bvh_node& world, const std::vector<shared_ptr<hittable>>& lightsource) {
	double RR = 0.98;
	double random_number = random_double();
	if (random_number > RR) return Color3d(0.0, 0.0, 0.0);

	hit_record rec;
	scatter_record srec;

	if (!world.hit(r, 0.001, infinity, rec)) return Color3d(0.0, 0.0, 0.0);

	Color3d emitted = rec.mat_ptr->emitted(rec);
	if (!rec.mat_ptr->scatter(r, rec, srec))
		return emitted / RR;

	auto light0 = make_shared<hittable_pdf>(rec.p, lightsource[0]);
	auto light1 = make_shared<hittable_pdf>(rec.p, lightsource[1]);
	mixture_pdf m_pdf(srec.pdf_ptr, light0, light1);

	ray scatter_ray = ray(rec.p, m_pdf.generate());
	return emitted + 
		srec.attenuation.cwiseProduct(ray_color(scatter_ray, world, lightsource)) * rec.n.dot(scatter_ray.dir.normalized()) * rec.mat_ptr->scattering_pdf(r, rec, scatter_ray)
		/ (RR * m_pdf.value(scatter_ray.dir));
}

class tile {
public:
	tile(int w_b, int w_e, int h_b, int h_e, int i_w, int i_h) {
		width_begin = w_b;
		width_end = w_e;
		height_begin = h_b;
		height_end = h_e;
		image_width = i_w;
		image_height = i_h;
		color_block = new Color3d[(width_end - width_begin) * (height_end - height_begin)];
	}

	void render(bvh_node& bvh, camera& cam, std::vector<shared_ptr<hittable>>& lightsource, int samples_per_pixel);
public:
	static int cores_left;
	int image_width;
	int image_height;
	int width_begin;
	int width_end;
	int height_begin;
	int height_end;
	Color3d* color_block;
};

int tile::cores_left;

void thread_function(tile& t, bvh_node& bvh, camera& cam, std::vector<shared_ptr<hittable>>& lightsource, int samples_per_pixel) {
	for (int j = t.height_end - 1; j >= t.height_begin; --j) {
		//std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
		for (int i = t.width_begin; i < t.width_end; ++i) {
			Color3d pixel_color(0, 0, 0);
			for (int s = 0; s < samples_per_pixel; ++s) {
				auto u = (i + random_double()) / (t.image_width - 1);
				auto v = (j + random_double()) / (t.image_height - 1);
				ray r = cam.ray_generate(u, v);
				pixel_color += ray_color(r, bvh, lightsource);
			}
			t.color_block[(j - t.height_begin) * (t.width_end - t.width_begin) + (i - t.width_begin)] = pixel_color;
		}
	}
	change.lock();
	t.cores_left = t.cores_left + 1;
	change.unlock();

	//std::cerr << "thread has quit.\n";
}

void tile::render(bvh_node& bvh, camera& cam, std::vector<shared_ptr<hittable>>& lightsource, int samples_per_pixel) {
	//std::cerr << "tile has been created." << std::endl;
	change.lock();
	cores_left = cores_left - 1;
	std::thread rendering(thread_function, std::ref(*this), std::ref(bvh), std::ref(cam), std::ref(lightsource), samples_per_pixel);
	rendering.detach();
	change.unlock();
}