#pragma once
#include "myraytracer.h"
#include "hittable.h"
#include "ray.h"
#include "material.h"

class triangle : public hittable{
public:
	triangle() {}
	triangle(Point4d v0, Point4d v1, Point4d v2, shared_ptr<material> mat) {
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
	virtual double pdf_value(const Point4d& o, const Vector4d& v);
	virtual Vector4d generate_direction(const Point4d& o);

	void set_vertex(Point4d v0, Point4d v1, Point4d v2) {
		v[0] = v0; v[1] = v1; v[2] = v2;
	}
	void set_normal(Vector4d n0, Vector4d n1, Vector4d n2) {
		n[0] = n0; n[1] = n1; n[2] = n2;
	}
	void set_texcord(Texture2d t0, Texture2d t1, Texture2d t2) {
		tex[0] = t0; tex[1] = t1; tex[2] = t2;
	}
	void set_material(shared_ptr<material> mat) {
		mat_ptr = mat;
	}

	virtual void apply_vertex_transformation(Eigen::Matrix4d& matrix) {
		v[0] = matrix * v[0];
		v[1] = matrix * v[1];
		v[2] = matrix * v[2];
	}

	virtual void apply_normal_transformation(Eigen::Matrix4d& matrix) {
		n[0] = matrix * n[0];
		n[1] = matrix * n[1];
		n[2] = matrix * n[2];
	}
public:
	Vector4d v[3];
	Vector4d n[3];
	Texture2d tex[3];
	shared_ptr<material> mat_ptr;
private:
	std::tuple<double, double, double> isInTriangle(const Point4d& p) const;
};

bool triangle::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
	/*�㷨ʽ���̿��ٽⷨ
	t = N * (T - origin) / (N * dir)
	����t�ǹ��߷����е�ʱ�������
	N�������εķ��ߣ�T�������ε�����һ�����㣬origin�ǹ��߷����й��ߵĳ����㣬dir�ǹ��߷����й��ߵķ���
	���Խ��������ƽ��Ľ��㣬���������
	*/
	auto e0 = v[1] - v[0];
	auto e1 = v[2] - v[0];
	//���ǵ���ģ���е���������η��߿��ܲ�������������ƽ��ķ��ߣ���Ҫ�ֶ�����ƽ�淨��
	auto normal = cross(e0, e1);

	auto denominator = normal.dot(r.dir);
	if (denominator == 0) return false;//������ƽ��ƽ�У��������н���

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
	if (denominator == 0) {//�㷨�Ǵ�rasterizer���������ģ�ֻ�����˶�ά�������
		//����������xyƽ���ϵ�ͶӰ����һ�������ε�ʱ�����������ʧЧ��
		//�����������������һ����ֱ��xyƽ���ֱ����
		//���Կ��ǻ�һ����ͶӰ���������ø��㷨
		if (v[0].x() == v[1].x() && v[0].x() == v[2].x()) {//��������yzƽ����
			auto mm0 = m3;//mm0 = v[1].y() - v[0].y()
			auto mm1 = v[2].z() - v[0].z();
			auto mm2 = m1;//mm2 = v[2].y() - v[0].y()
			auto mm3 = v[1].z() - v[0].z();
			auto mm4 = m5;//mm4 = p.y() - v[0].y()
			auto mm5 = p.z() - v[0].z();
			//��ʱdenominator�Ѿ�������Ϊ0
			denominator = (mm0 * mm1) - (mm2 * mm3);
			auto denominator_inv = 1.0 / denominator;
			auto beta = ((mm4 * mm1) - (mm2 * mm5)) * denominator_inv;
			auto gamma = ((mm0 * mm5) - (mm4 * mm3)) * denominator_inv;
			auto alpha = 1 - beta - gamma;
			return { alpha, beta, gamma };
		}
		else if (v[0].y() == v[1].y() && v[0].y() == v[2].y()) {//��������xzƽ����
			auto mm0 = m0;//mm0 = v[1].x() - v[0].x()
			auto mm1 = v[2].z() - v[0].z();
			auto mm2 = m2;//mm2 = v[2].x() - v[0].x()
			auto mm3 = v[1].z() - v[0].z();
			auto mm4 = m4;//mm4 = p.x() - v[0].x()
			auto mm5 = p.z() - v[0].z();
			//��ʱdenominator�Ѿ�������Ϊ0
			denominator = (mm0 * mm1) - (mm2 * mm3);
			auto denominator_inv = 1.0 / denominator;
			auto beta = ((mm4 * mm1) - (mm2 * mm5)) * denominator_inv;
			auto gamma = ((mm0 * mm5) - (mm4 * mm3)) * denominator_inv;
			auto alpha = 1 - beta - gamma;
			return { alpha, beta, gamma };
		}
		else {//ֻ�������������ڴ�ֱ��xyƽ���ֱ����
			//���Խ���ͶӰ��yzƽ���xzƽ��
			//����ѡ����ͶӰ��yzƽ��
			auto mm0 = m3;//mm0 = v[1].y() - v[0].y()
			auto mm1 = v[2].z() - v[0].z();
			auto mm2 = m1;//mm2 = v[2].y() - v[0].y()
			auto mm3 = v[1].z() - v[0].z();
			auto mm4 = p.y() - v[0].y();
			auto mm5 = p.z() - v[0].z();
			//��ʱdenominator�Ѿ�������Ϊ0
			denominator = (mm0 * mm1) - (mm2 * mm3);
			auto denominator_inv = 1.0 / denominator;
			auto beta = ((mm4 * mm1) - (mm2 * mm5)) * denominator_inv;
			auto gamma = ((mm0 * mm5) - (mm4 * mm3)) * denominator_inv;
			auto alpha = 1 - beta - gamma;
			return { alpha, beta, gamma };
		}
	}

	auto denominator_inv = 1.0 / denominator;
	auto beta = ((m4 * m1) - (m2 * m5)) * denominator_inv;
	auto gamma = ((m0 * m5) - (m4 * m3)) * denominator_inv;
	auto alpha = 1 - beta - gamma;
	return { alpha, beta, gamma }; 
}

bool triangle::bounding_box(b_box& output_box) const {
	auto x_min = fmin(v[0].x(), fmin(v[1].x(), v[2].x()));
	auto y_min = fmin(v[0].y(), fmin(v[1].y(), v[2].y()));
	auto z_min = fmin(v[0].z(), fmin(v[1].z(), v[2].z()));
	auto x_max = fmax(v[0].x(), fmax(v[1].x(), v[2].x()));
	auto y_max = fmax(v[0].y(), fmax(v[1].y(), v[2].y()));
	auto z_max = fmax(v[0].z(), fmax(v[1].z(), v[2].z()));
	
	output_box.minimum = Point4d(x_min - 0.001, y_min - 0.001, z_min - 0.001, 1.0);
	output_box.maximum = Point4d(x_max + 0.001, y_max + 0.001, z_max + 0.001, 1.0);
	return true;
}

inline double triangle::pdf_value(const Point4d& o, const Vector4d& dir) {
	ray r(o, dir); hit_record rec;
	if (!hit(r, 0.001, infinity, rec)) return 0;
	else {
		auto e0 = v[1] - v[0];
		auto e1 = v[2] - v[0];
		auto vec = cross(e0, e1);
		auto area = sqrt(vec.dot(vec)) / 2.0;
		auto cosine = rec.n.dot(-dir);
		auto distance_sq = (rec.p - o).dot(rec.p - o);
		auto value = distance_sq / (area * cosine);
		if (value < 1 / (2 * pi)) return 1 / (2 * pi);
		else return value;
	}
}

inline Vector4d triangle::generate_direction(const Point4d& o) {
	auto xi_1 = random_double();
	auto xi_2 = random_double();
	Point4d p = (1 - sqrt(xi_1)) * v[0] + (sqrt(xi_1) * (1 - xi_2)) * v[1] + sqrt(xi_1) * xi_2 * v[2];
	auto dir = p - o;
	return dir.normalized();
}