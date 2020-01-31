#pragma once
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>
#include <Eigen/Core>
#include <Eigen/LU>
#include <Eigen/Eigenvalues>
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
	double factor; // With what exp is multiplied: 1/((2pi)^(3/2) * det(sigma)^(1/2)
	Eigen::Vector3d mu;
	QMatrix3x3 eigenmatrix;

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
		Eigen::EigenSolver<Eigen::Matrix3Xd> eigensolver;
		eigensolver.compute(covariancematrix, true);
		Eigen::VectorXd eigen_values = eigensolver.eigenvalues().real();
		Eigen::MatrixXd eigen_vectors = eigensolver.eigenvectors().real();
		float l0 = sqrt(eigen_values(0));
		float l1 = sqrt(eigen_values(1));
		float l2 = sqrt(eigen_values(2));
		float values[9] = { l0 * eigen_vectors(0, 0), l1 * eigen_vectors(0, 1), l2 * eigen_vectors(0, 2),
			l0 * eigen_vectors(1, 0), l1 * eigen_vectors(1, 1), l2 * eigen_vectors(1, 2),
			l0 * eigen_vectors(2, 0), l1 * eigen_vectors(2, 1), l2 * eigen_vectors(2, 2) };
		eigenmatrix = QMatrix3x3(values);
	}

	double sample(double x, double y, double z) const {
		Eigen::Vector3d relpos = Eigen::Vector3d(x - this->x, y - this->y, z - this->z);
		double ex = std::exp(-0.5 * (relpos.transpose() * inversecovariance * relpos).x());
		float val = factor * weight * ex;
		return val;
	}

	QMatrix3x3 getEigenMatrix() const {
		return eigenmatrix;
	}

	void getBoundingBox(double threshold, QVector3D& min, QVector3D& max) const {
		float scalar = sqrt(-2 * log(threshold / gpudata.mu_amplitude.w()));
		QMatrix3x3 transfo = eigenmatrix * scalar;
		QVector3D r0 = QVector3D(transfo(0, 0), transfo(0, 1), transfo(0, 2));
		QVector3D r1 = QVector3D(transfo(1, 0), transfo(1, 1), transfo(1, 2));
		QVector3D r2 = QVector3D(transfo(2, 0), transfo(2, 1), transfo(2, 2));
		QVector3D delta = QVector3D(r0.length(), r1.length(), r2.length());
		QVector3D muq = QVector3D(mu.x(), mu.y(), mu.z());
		min = muq - delta;
		max = muq + delta;
	}
};