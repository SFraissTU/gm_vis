#include "GaussianMixture.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <Eigen/Core>

double GaussianMixture::sample(double x, double y, double z)
{
	Eigen::Vector3d pos = Eigen::Vector3d(x, y, z);
	double sum = 0.0f;
	double divisor = 1.0 / pow(2 * M_PI, 3.0 / 2.0);
	for (int i = 0; i < gaussians.size(); ++i) {
		Gaussian& gauss = gaussians[i];
		Eigen::Vector3d relpos = pos - gauss.mu;
		double factor = std::exp(-0.5 * (relpos.transpose() * gauss.inversecovariance * relpos).x());
		sum += divisor * gauss.piinvsqrtcovdeterminant * factor;
	}
	return sum;
}
