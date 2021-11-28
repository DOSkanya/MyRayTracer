#include <iostream>
#include <thread>
#include "myraytracer.h"
#include "triangle.h"
#include "sphere.h"
#include "camera.h"
#include "color.h"
#include "hittable_list.h"
#include "bvh_tree.h"
#include "tile.h"

std::mutex change;

const int screen_height = 200;
const int screen_width = 200;
const int samples_per_pixel = 50;

Color3d ray_color(const ray& r, bvh_node& world, int depth);
Color3d* pixel_color;

int main() {
	pixel_color = new Color3d[screen_height * screen_width];
	for (int h = 0; h < screen_width * screen_height; h++)
		pixel_color[h] = Color3d(0.0, 0.0, 0.0);

	Point4d v0, v1, v2, v3, v4, v5, v6, v7;
	v0 << 0.0, 0.0, 0.0, 1.0;
	v1 << 500.0, 0.0, 0.0, 1.0;
	v2 << 0.0, 500.0, 0.0, 1.0;
	v3 << 500.0, 500.0, 0.0, 1.0;
	v4 << 0.0, 0.0, 500.0, 1.0;
	v5 << 500.0, 0.0, 500.0, 1.0;
	v6 << 0.0, 500.0, 500.0, 1.0;
	v7 << 500.0, 500.0, 500.0, 1.0;
	auto red = make_shared<lambertian>(Color3d(.65, .05, .05));
	auto green = make_shared<lambertian>(Color3d(.12, .45, .15));
	auto white = make_shared<lambertian>(Color3d(.73, .73, .73));
	//left wall
	shared_ptr<triangle> t0 = make_shared<triangle>(v0, v2, v4, green);
	shared_ptr<triangle> t1 = make_shared<triangle>(v2, v4, v6, green);
	//right wall
	shared_ptr<triangle> t2 = make_shared<triangle>(v1, v3, v5, red);
	shared_ptr<triangle> t3 = make_shared<triangle>(v3, v5, v7, red);
	//top wall
	shared_ptr<triangle> t4 = make_shared<triangle>(v2, v3, v6, white);
	shared_ptr<triangle> t5 = make_shared<triangle>(v3, v6, v7, white);
	//bottom wall
	shared_ptr<triangle> t6 = make_shared<triangle>(v0, v1, v4, white);
	shared_ptr<triangle> t7 = make_shared<triangle>(v1, v4, v5, white);
	//inner wall
	shared_ptr<triangle> t8 = make_shared<triangle>(v4, v5, v6, white);
	shared_ptr<triangle> t9 = make_shared<triangle>(v5, v6, v7, white);

	hittable_list world;
	world.add(t0);
	world.add(t1);
	world.add(t2);
	world.add(t3);
	world.add(t4);
	world.add(t5);
	world.add(t6);
	world.add(t7);
	world.add(t8);
	world.add(t9);
	
	Point4d o;
	o << 250.0, 250.0, 250.0, 1.0;
	shared_ptr<sphere> light = make_shared<sphere>(o, 50, make_shared<diffuse_light>(Color3d(2.0, 2.0, 2.0)));
	world.add(light);

	bvh_node root(world);

	/*Camera parameter*/
	Point4d lookfrom(250.0, 250.0, -250.0, 1.0);
	Point4d lookat(250.0, 250.0, 1.0, 1.0);
	Vector4d vup(0.0, 1.0, 0.0, 0.0);
	double vfov = 90.0;
	double aspect_ratio = (double)screen_width / (double)screen_height;
	double focus_dist = 1.0;

	camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, focus_dist);
	
	//Parallelize: Get threads that can be concurrently executed
	int _threadNum = std::thread::hardware_concurrency();
	tile::cores_left = _threadNum;

	//Render
	int tile_width = 0, tile_height = 0;
	int tile_scale = 100;
	std::vector<tile*> tile_array;
	while (tile_width < screen_width && tile_height < screen_height) {
		if (tile::cores_left > 0) {
			if ((tile_width + tile_scale) >= screen_width && (tile_height + tile_scale) >= screen_height) {
				tile* t = new tile(tile_width, screen_width, tile_height, screen_height, screen_width, screen_height);
				tile_array.push_back(t);
				t->render(root, cam, samples_per_pixel);
				tile_width = 0;
				tile_height = tile_height + tile_scale;
			}
			else if ((tile_width + tile_scale) >= screen_width && (tile_height + tile_scale) < screen_height) {
				tile* t = new tile(tile_width, screen_width, tile_height, tile_height + tile_scale, screen_width, screen_height);
				tile_array.push_back(t);
				t->render(root, cam, samples_per_pixel);
				tile_width = 0;
				tile_height = tile_height + tile_scale;
			}
			else if ((tile_width + tile_scale) < screen_width && (tile_height + tile_scale) >= screen_height) {
				tile* t = new tile(tile_width, tile_width + tile_scale, tile_height, screen_height, screen_width, screen_height);
				tile_array.push_back(t);
				t->render(root, cam, samples_per_pixel);
				tile_width = tile_width + tile_scale;
			}
			else if ((tile_width + tile_scale) < screen_width && (tile_height + tile_scale) < screen_height) {
				tile* t = new tile(tile_width, tile_width + tile_scale, tile_height, tile_height + tile_scale, screen_width, screen_height);
				tile_array.push_back(t);
				t->render(root, cam, samples_per_pixel);
				tile_width = tile_width + tile_scale;
			}
		}
	}

	bool rendering_fin = false;
	while (!rendering_fin) {
		if (tile::cores_left == _threadNum)
			rendering_fin = true;
	}

	//Output
	Color3d* pixel_color = new Color3d[screen_width * screen_height];
	for (auto ti : tile_array) {
		for (int j = ti->height_end - 1; j >= ti->height_begin; --j) {
			for (int i = ti->width_begin; i < ti->width_end; i++) {
				pixel_color[(screen_height - 1 - j) * screen_width + i] = ti->color_block[(j - ti->height_begin) * (ti->width_end - ti->width_begin) + (i - ti->width_begin)];
			}
		}
	}

	std::cout << "P3\n" << screen_width << " " << screen_height << "\n255\n";
	for (int b = 0; b < screen_height * screen_width; b++) {
		write_color(std::cout, pixel_color[b], samples_per_pixel);
	}

	delete [] pixel_color;
	return 0;
}

Color3d ray_color(const ray& r, bvh_node& world, int depth) {
	hit_record rec;
	scatter_record srec;

	if (depth <= 0) return Color3d(0.0, 0.0, 0.0);
	if (!world.hit(r, 0.001, infinity, rec)) return Color3d(0.0, 0.0, 0.0);

	Color3d emitted = rec.mat_ptr->emitted(rec);
	if (!rec.mat_ptr->scatter(r, rec, srec))
		return emitted;

	rec.mat_ptr->scatter(r, rec, srec);//Compute the scatter ray then stored in srec
	return emitted + srec.attenuation.cwiseProduct(ray_color(srec.scatter_ray, world, depth - 1));
}