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
};