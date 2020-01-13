#pragma once
#include <QVector>
#include "Gaussian.h"

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
};