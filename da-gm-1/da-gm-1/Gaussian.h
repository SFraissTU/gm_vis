#pragma once
#include <QMatrix3x3>
#include <Eigen/Core>
#include <Eigen/LU>

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

	Eigen::Matrix3d covariancematrix;
	Eigen::Matrix3d inversecovariance;
	double piinvsqrtcovdeterminant;
	Eigen::Vector3d mu;

	void finalizeInitialization() {
		covariancematrix(0, 0) = covxx;
		covariancematrix(0, 1) = covariancematrix(1, 0) = covxy;
		covariancematrix(0, 2) = covariancematrix(2, 0) = covxz;
		covariancematrix(1, 1) = covyy;
		covariancematrix(1, 2) = covariancematrix(2, 1) = covyz;
		covariancematrix(2, 2) = covzz;
		inversecovariance = covariancematrix.inverse();
		piinvsqrtcovdeterminant = weight / std::sqrt(covariancematrix.determinant());
		mu = Eigen::Vector3d(x, y, z);
	}
};