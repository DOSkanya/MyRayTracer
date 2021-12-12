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
#include "OBJ_Loader.h"

std::mutex change;

const int screen_height = 200;
const int screen_width = 200;
const int samples_per_pixel = 100;

Color3d* pixel_color;
//Color3d ray_color(const ray& r, const bvh_node& world);

int main() {
	pixel_color = new Color3d[screen_height * screen_width];
	for (int h = 0; h < screen_width * screen_height; h++)
		pixel_color[h] = Color3d(0.0, 0.0, 0.0);

	auto red = make_shared<lambertian>(Color3d(.65, .05, .05));
	auto green = make_shared<lambertian>(Color3d(.12, .45, .15));
	auto white = make_shared<lambertian>(Color3d(.73, .73, .73));
	auto light = make_shared<diffuse_light>(Color3d(25, 25, 25));
	auto yellow = make_shared<lambertian>(Color3d(.45, .45, .10));

	objl::Loader loader;
	loader.LoadFile("resource/african_head/african_head.obj");
	hittable_list obj_model;
	for (int i = 0; i < loader.LoadedVertices.size(); i += 3) {
		Point4d v_1, v_2, v_3;
		Vector4d n_1, n_2, n_3;
		Texture2d t_1, t_2, t_3;
		v_1 << loader.LoadedVertices[i].Position.X, loader.LoadedVertices[i].Position.Y, loader.LoadedVertices[i].Position.Z, 1.0;
		v_2 << loader.LoadedVertices[i + 1].Position.X, loader.LoadedVertices[i + 1].Position.Y, loader.LoadedVertices[i + 1].Position.Z, 1.0;
		v_3 << loader.LoadedVertices[i + 2].Position.X, loader.LoadedVertices[i + 2].Position.Y, loader.LoadedVertices[i + 2].Position.Z, 1.0;
		n_1 << loader.LoadedVertices[i].Normal.X, loader.LoadedVertices[i].Normal.Y, loader.LoadedVertices[i].Normal.Z, 0.0;
		n_2 << loader.LoadedVertices[i + 1].Normal.X, loader.LoadedVertices[i + 1].Normal.Y, loader.LoadedVertices[i + 1].Normal.Z, 0.0;
		n_3 << loader.LoadedVertices[i + 2].Normal.X, loader.LoadedVertices[i + 2].Normal.Y, loader.LoadedVertices[i + 2].Normal.Z, 0.0;
		t_1 << loader.LoadedVertices[i].TextureCoordinate.X, loader.LoadedVertices[i].TextureCoordinate.Y;
		t_2 << loader.LoadedVertices[i + 1].TextureCoordinate.X, loader.LoadedVertices[i + 1].TextureCoordinate.Y;
		t_3 << loader.LoadedVertices[i + 2].TextureCoordinate.X, loader.LoadedVertices[i + 2].TextureCoordinate.Y;
		shared_ptr<triangle> t = make_shared<triangle>();
		t->set_vertex(v_1, v_2, v_3);
		t->set_normal(n_1, n_2, n_3);
		t->set_texcord(t_1, t_2, t_3);
		t->mat_ptr = white;
		obj_model.add(t);
	}
	//由于标准化空间是从-1到1，跨度为2，所以正确的缩放参数是想要缩放比例的二分之一
	obj_model.apply_transformation(140.0, 160.0, Vector3d(140.0 + 25.0, 140.0 + 0.0, 140.0 + 170.0));

	shared_ptr<bvh_node> model = make_shared<bvh_node>(obj_model);

	Point4d v0, v1, v2, v3, v4, v5, v6, v7;
	v0 << 0.0, 0.0, 0.0, 1.0;
	v1 << 500.0, 0.0, 0.0, 1.0;
	v2 << 0.0, 500.0, 0.0, 1.0;
	v3 << 500.0, 500.0, 0.0, 1.0;
	v4 << 0.0, 0.0, 500.0, 1.0;
	v5 << 500.0, 0.0, 500.0, 1.0;
	v6 << 0.0, 500.0, 500.0, 1.0;
	v7 << 500.0, 500.0, 500.0, 1.0;
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
	//outer wall
	//shared_ptr<triangle> t10 = make_shared<triangle>(v0, v1, v2, white);
	//shared_ptr<triangle> t11 = make_shared<triangle>(v1, v2, v3, white);
	//light
	Point4d v8, v9, v10, v11;
	v8 << 200.0, 499.0, 200.0, 1.0;
	v9 << 200.0, 499.0, 300.0, 1.0;
	v10 << 300.0, 499.0, 200.0, 1.0;
	v11 << 300.0, 499.0, 300.0, 1.0;
	shared_ptr<triangle> t12 = make_shared<triangle>(v8, v9, v10, light);
	shared_ptr<triangle> t13 = make_shared<triangle>(v9, v10, v11, light);

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
	//world.add(t10);
	//world.add(t11);
	world.add(t12);
	world.add(t13);
	world.add(model);

	std::vector<shared_ptr<hittable>> lightsource;
	lightsource.push_back(t12);
	lightsource.push_back(t13);

	/*
	Point4d o;
	o << 250.0, 250.0, 250.0, 1.0;
	shared_ptr<sphere> light = make_shared<sphere>(o, 50, make_shared<diffuse_light>(Color3d(10.0, 10.0, 10.0)));
	world.add(light);
	*/

	bvh_node root(world);

	/*Camera parameter*/
	Point4d lookfrom(250.0, 250.0, -725.0, 1.0);
	Point4d lookat(250.0, 250.0, 1.0, 1.0);
	Vector4d vup(0.0, 1.0, 0.0, 0.0);
	double vfov = 40.0;
	double aspect_ratio = (double)screen_width / (double)screen_height;
	double focus_dist = 1.0;

	camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, focus_dist);
	
	
	//Parallelize: Get threads that can be concurrently executed
	int _threadNum = std::thread::hardware_concurrency();
	tile::cores_left = _threadNum;

	//Render
	int tile_width = 0, tile_height = 0;
	int tile_scale = 50;
	std::vector<tile*> tile_array;
	while (tile_width < screen_width && tile_height < screen_height) {
		if (tile::cores_left > 0) {
			if ((tile_width + tile_scale) >= screen_width && (tile_height + tile_scale) >= screen_height) {
				tile* t = new tile(tile_width, screen_width, tile_height, screen_height, screen_width, screen_height);
				tile_array.push_back(t);
				t->render(root, cam, lightsource, samples_per_pixel);
				tile_width = 0;
				tile_height = tile_height + tile_scale;
			}
			else if ((tile_width + tile_scale) >= screen_width && (tile_height + tile_scale) < screen_height) {
				tile* t = new tile(tile_width, screen_width, tile_height, tile_height + tile_scale, screen_width, screen_height);
				tile_array.push_back(t);
				t->render(root, cam, lightsource, samples_per_pixel);
				tile_width = 0;
				tile_height = tile_height + tile_scale;
			}
			else if ((tile_width + tile_scale) < screen_width && (tile_height + tile_scale) >= screen_height) {
				tile* t = new tile(tile_width, tile_width + tile_scale, tile_height, screen_height, screen_width, screen_height);
				tile_array.push_back(t);
				t->render(root, cam, lightsource, samples_per_pixel);
				tile_width = tile_width + tile_scale;
			}
			else if ((tile_width + tile_scale) < screen_width && (tile_height + tile_scale) < screen_height) {
				tile* t = new tile(tile_width, tile_width + tile_scale, tile_height, tile_height + tile_scale, screen_width, screen_height);
				tile_array.push_back(t);
				t->render(root, cam, lightsource, samples_per_pixel);
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
	
	/*
	for (int j = screen_height - 1; j >= 0; --j) {
		//std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
		for (int i = 0; i < screen_width; ++i) {
			Color3d color(0, 0, 0);
			for (int s = 0; s < samples_per_pixel; ++s) {
				auto u = (i + random_double()) / (screen_width - 1);
				auto v = (j + random_double()) / (screen_height - 1);
				ray r = cam.ray_generate(u, v);
				color += ray_color(r, world);
			}
			pixel_color[(screen_height - j - 1) * screen_width + i] = color;
		}
	}*/
	

	std::cout << "P3\n" << screen_width << " " << screen_height << "\n255\n";
	for (int b = 0; b < screen_height * screen_width; b++) {
		write_color(std::cout, pixel_color[b], samples_per_pixel);
	}

	delete [] pixel_color;
	return 0;
}

/*
Color3d ray_color(const ray& r, const bvh_node& world) {
	double RR = 0.98;
	double random_number = random_double();
	if (random_number > RR) return Color3d(0.0, 0.0, 0.0);

	hit_record rec;
	scatter_record srec;

	if (!world.hit(r, 0.001, infinity, rec)) return Color3d(0.0, 0.0, 0.0);

	Color3d emitted = rec.mat_ptr->emitted(rec);
	if (!rec.mat_ptr->scatter(r, rec, srec))
		return emitted / RR;

	ray scatter_ray = ray(rec.p, srec.pdf_ptr->generate());
	return emitted + srec.attenuation.cwiseProduct(ray_color(scatter_ray, world) * rec.n.dot(scatter_ray.dir.normalized())) / RR;
}
*/