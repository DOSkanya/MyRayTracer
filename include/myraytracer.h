#pragma once
#include <Eigen/Core>
#include <Eigen/Dense>
#include <random>

const double infinity = std::numeric_limits<double>::infinity();

typedef Eigen::Vector3d Vector3d;
typedef Eigen::Vector3d Point3d;
typedef Eigen::Vector3d Color3d;
typedef Eigen::Vector4d Vector4d;
typedef Eigen::Vector4d Point4d;
typedef Eigen::Vector2d Vector2d;
typedef Eigen::Vector2d Texture2d;

using std::shared_ptr;
using std::make_shared;
using std::sqrt;

const double pi = 3.141592653;

double degrees_to_radians(double degrees) {
	return degrees * pi / 180;
}

double clamp(double x, double min, double max) {
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

Vector4d cross(const Vector4d& v1, const Vector4d& v2) {
	Vector3d v1_3d = v1.segment(0, 3);
	Vector3d v2_3d = v2.segment(0, 3);
	Vector3d temp = v1_3d.cross(v2_3d);
	Vector4d result;
	result << temp.x(), temp.y(), temp.z(), 0.0f;
	return result;
}

inline double random_double() {
	static std::uniform_real_distribution<double> distribution(0.0, 1.0);
	static std::mt19937_64 generator;
	return distribution(generator);
}

inline double random_double(double min, double max) {
	return min + (max - min) * random_double();
}

inline Vector4d random_vector() {
	return Vector4d(random_double(), random_double(), random_double(), 0.0);
}

inline Vector4d random_vector(double min, double max) {
	return Vector4d(random_double(min, max), random_double(min, max), random_double(min, max), 0.0);
}

inline Vector4d random_in_unit_sphere() {
	while (true) {
		auto p = random_vector(-1, 1);
		if (p.dot(p) >= 1) continue;
		return p;
	}
}

inline Vector4d random_unit_vector() {
	return random_vector(-1, 1).normalized();
}