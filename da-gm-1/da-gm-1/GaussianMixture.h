#pragma once
#include <QVector>
#include <QOpenGLFunctions_4_5_Core>
#include "Gaussian.h"

struct GMOctreeNode {
	QVector4D min;
	QVector4D max;
	GLint childrenbits;
	GLint childrenstart;
	GLint gaussianstart;
	GLint gaussianend;
};

class GaussianMixture {
private:
	QVector<Gaussian> gaussians;

public:

	void addGaussian(const Gaussian& gauss) {
		gaussians.push_back(gauss);
	}

	int numberOfGaussians() const {
		return gaussians.size();
	}

	double sample(double x, double y, double z) const;

	const Gaussian* operator[](int index) const {
		if (index >= gaussians.size() || index < 0) {
			return nullptr;
		}
		else {
			return &gaussians[index];
		}
	}

	std::shared_ptr<char[]> gpuData(size_t& arrsize) const;

	//Returns the Gaussians-Data as return value and the octree nodes per parameter
	std::shared_ptr<char[]> buildOctree(double threshold, QVector<GMOctreeNode>& result, size_t& arrsize) const;

	void normalize();
};