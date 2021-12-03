#pragma once
#include "myraytracer.h"
#include "ortho.h"

class pdf {
public:
	virtual ~pdf() {}
	virtual double value(const Vector4d& direction) const = 0;
	virtual Vector4d generate() const = 0;
};

class uniform_pdf : public pdf {
public:
	uniform_pdf(const Vector4d& normal) {
		uvw.build_from_vector(normal);
	}
	virtual double value(const Vector4d& direction) const;
	virtual Vector4d generate() const override;
public:
	ortho uvw;
};

double uniform_pdf::value(const Vector4d& direction) const {
	return 1 / (2 * pi);
}

Vector4d uniform_pdf::generate() const {
	//PBR�Ĳ��������ǻ��ھ�����������ϵ,xy��λ��ƽ���ϣ���z���������
	//��Ⱦ���ﶨ���xyz��ͼ����г��õ�����ϵ��һ����Ҫע��Ӧ�ó���
	auto xi_1 = random_double();
	auto xi_2 = random_double();
	auto sqrt_value = std::sqrt(1 - xi_1 * xi_1);

	auto x = cos(2 * pi * xi_2) * sqrt_value;
	auto y = xi_1;
	auto z = sin(2 * pi * xi_2) * sqrt_value;
	return uvw.local(x, y, z);
}