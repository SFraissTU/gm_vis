#include "GaussianMixture.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <Eigen/Core>
#include <QOpenGLFunctions_4_5_Core>

double GaussianMixture::sample(double x, double y, double z) const
{
	Eigen::Vector3d pos = Eigen::Vector3d(x, y, z);
	double sum = 0.0f;
	for (int i = 0; i < gaussians.size(); ++i) {
		//if (i != 4) continue;
		const Gaussian& gauss = gaussians[i];
		sum += gauss.sample(x, y, z);
	}
	return sum;
}

std::shared_ptr<char[]> GaussianMixture::gpuData(size_t& arrsize) const
{
	GLint n = (GLint)numberOfGaussians();
	arrsize = sizeof(GLint)*4 + 80 * n;
	char* result = new char[arrsize];
	memcpy(result, &n, sizeof(GLint));
	char* gaussmem = result + sizeof(GLint)*4;
	for (int i = 0; i < n; i++) {
		GaussianGPU gpudata = gaussians[i].gpudata;
		memcpy(gaussmem, &gpudata.mu_amplitude, 16);
		gaussmem += 16;
		memcpy(gaussmem, gpudata.invsigma.constData(), 64);
		gaussmem += 64;
	}
	return std::shared_ptr<char[]>(result);
}
