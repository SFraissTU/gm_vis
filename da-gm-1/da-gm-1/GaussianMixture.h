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

	const Gaussian* operator[](int index) {
		if (index >= gaussians.size() || index <= 0) {
			return nullptr;
		}
		else {
			return &gaussians[index];
		}
	}
};