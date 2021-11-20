#pragma once
#include "myraytracer.h"
#include "hittable.h"
#include "ray.h"
#include "material.h"

class triangle : public hittable{
public:
	triangle() {}
	triangle(Vector4d v0, Vector4d v1, Vector4d v2, shared_ptr<material> mat) {
		v[0] = v0;
		v[1] = v1;
		v[2] = v2;
		auto e0 = v[1] - v[0];
		auto e1 = v[2] - v[0];
		n[0] = n[1] = n[2] = cross(e0, e1).normalized();
		tex[0] = tex[1] = tex[2] = Texture2d(0.0f, 0.0f);
		mat_ptr = mat;
	}

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
	virtual bool bounding_box(b_box& output_box) const override;

	Vector4d v[3];
	Vector4d n[3];
	Texture2d tex[3];
	shared_ptr<material> mat_ptr;
private:
	std::tuple<double, double, double> isInTriangle(const Point4d& p) const;
};

bool triangle::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
	/*点法式方程快速解法
	t = N * (T - origin) / (N * dir)
	其中t是光线方程中的时间变量，
	N是三角形的法线，T是三角形的任意一个顶点，origin是光线方程中光线的出发点，dir是光线方程中光线的方向
	可以解出光线与平面的交点，简洁又优雅
	*/
	auto e0 = v[1] - v[0];
	auto e1 = v[2] - v[0];
	//考虑到从模型中导入的三角形法线可能不是三角形所在平面的法线，需要手动重算平面法线
	auto normal = cross(e0, e1);

	auto denominator = normal.dot(r.dir);
	if (denominator == 0) return false;//光线与平面平行，不可能有交点

	auto t = normal.dot(v[0] - r.orig) / normal.dot(r.dir);
	if (t < t_min || t > t_max) return false;

	Point4d p = r.orig + t * r.dir;
	auto [alpha, beta, gamma] = isInTriangle(p);
	if (alpha >= 0 && beta >= 0 && gamma >= 0) {
		rec.t = t;
		rec.p = p;
		rec.n = alpha * n[0] + beta * n[1] + gamma * n[2];
		rec.n.normalize();
		rec.tex = alpha * tex[0] + beta * tex[1] + gamma * tex[2];
		rec.set_face_normal(r, rec.n);
		rec.mat_ptr = mat_ptr;
		return true;
	}
	else return false;
}

std::tuple<double, double, double> triangle::isInTriangle(const Point4d& p) const {
	auto m0 = v[1].x() - v[0].x();
	auto m1 = v[2].y() - v[0].y();
	auto m2 = v[2].x() - v[0].x();
	auto m3 = v[1].y() - v[0].y();
	auto m4 = p.x() - v[0].x();
	auto m5 = p.y() - v[0].y();

	auto denominator = (m0 * m1) - (m2 * m3);
	if (denominator == 0) return { 1.0, 0.0, 0.0 };

	auto denominator_inv = 1.0 / denominator;
	auto beta = ((m4 * m1) - (m2 * m5)) * denominator_inv;
	auto gamma = ((m0 * m5) - (m4 * m3)) * denominator_inv;
	auto alpha = 1 - beta - gamma;
	return { alpha, beta, gamma }; 
}

bool triangle::bounding_box(b_box& output_box) const {
	return true;
}