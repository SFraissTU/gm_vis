#pragma once
#include <QVector4D>
#include <QMatrix4x4>
#include <Eigen/Core>
#include <Eigen/LU>
#define _USE_MATH_DEFINES
#include <Math.h>


const double GAUSS_PI_FACTOR = 1.0 / pow(2 * M_PI, 3.0 / 2.0);

struct GaussianGPU {
	QVector4D mu_amplitude;
	QMatrix4x4 invsigma;
	//alignas(16) float amplitude;
	//QVector3D padding;
};

struct Gaussian {
	double x;
	double y;
	double z;
	double covxx;
	double covxy;
	double covxz;
	double covyy;
	double covyz;
	double covzz;
	double weight;

	GaussianGPU gpudata;

private:
	Eigen::Matrix3d covariancematrix;
	Eigen::Matrix3d inversecovariance;
	double factor; // With what exp is multiplied: 1/((2pi)/(3/2) * det(sigma)^(1/2)
	Eigen::Vector3d mu;

public:
	void finalizeInitialization() {
		covariancematrix(0, 0) = covxx;
		covariancematrix(0, 1) = covariancematrix(1, 0) = covxy;
		covariancematrix(0, 2) = covariancematrix(2, 0) = covxz;
		covariancematrix(1, 1) = covyy;
		covariancematrix(1, 2) = covariancematrix(2, 1) = covyz;
		covariancematrix(2, 2) = covzz;
		inversecovariance = covariancematrix.inverse();
		factor = 1.0f / (sqrt(covariancematrix.determinant())) * GAUSS_PI_FACTOR;
		mu = Eigen::Vector3d(x, y, z);
		float covdata[16] = { (float)inversecovariance(0,0), (float)inversecovariance(0,1), (float)inversecovariance(0,2), 0, 
			(float)inversecovariance(1,0), (float)inversecovariance(1,1), (float)inversecovariance(1,2), 0, 
			(float)inversecovariance(2,0), (float)inversecovariance(2,1), (float)inversecovariance(2,2), 0, 
			0.0f, 0.0f, 0.0f, 1.0f };
		gpudata = { QVector4D((float)mu.x(), (float)mu.y(), (float)mu.z(), float(weight * factor)), QMatrix4x4(covdata) };
	}

	double sample(double x, double y, double z) const {
		Eigen::Vector3d relpos = Eigen::Vector3d(x - this->x, y - this->y, z - this->z);
		double ex = std::exp(-0.5 * (relpos.transpose() * inversecovariance * relpos).x());
		float val = factor * weight * ex;
		return val;
	}
};