#pragma once
#include "myraytracer.h"
#include "tgaimage.h"

class texture {
public:
	virtual Color3d value(double u, double v) = 0;
};

class solid_color : public texture {
public:
	solid_color(Color3d c) : color(c) {}
	virtual Color3d value(double u, double v) {
		return color;
	}
private:
	Color3d color;
};

class image_texture : public texture {
public:
	image_texture(const char* filename) {
		image.read_tga_file(filename);
	}
	virtual Color3d value(double u, double v) override;
public:
	TGAImage image;
};

Color3d image_texture::value(double u, double v) {
	u = clamp(u, 0.0, 1.0);
	v = 1.0 - clamp(v, 0.0, 1.0);

	auto i = static_cast<int>(u * image.get_width());
	auto j = static_cast<int>(v * image.get_height());

	if (i >= image.get_width()) i = image.get_width() - 1;
	if (j >= image.get_height()) j = image.get_height() - 1;

	const auto color_scale = 1.0 / 255.0;
	TGAColor pixel = image.get(i, j);
	Color3d result(pixel[2] * color_scale, pixel[1] * color_scale, pixel[0] * color_scale);

	return result;
}