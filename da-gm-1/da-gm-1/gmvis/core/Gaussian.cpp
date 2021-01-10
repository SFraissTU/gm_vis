#include "Gaussian.h"
using namespace gmvis::core;


Gaussian gmvis::core::Gaussian::createGaussian(const RawGaussian& original, bool normalizedWeight)
{
	//---- Calculation of the inverse covariance matrix ----
	//- Create an Eigen object for the normal covariance matrix
	Eigen::Matrix3d covariancematrix;
	covariancematrix(0, 0) = original.covxx;
	covariancematrix(0, 1) = covariancematrix(1, 0) = original.covxy;
	covariancematrix(0, 2) = covariancematrix(2, 0) = original.covxz;
	covariancematrix(1, 1) = original.covyy;
	covariancematrix(1, 2) = covariancematrix(2, 1) = original.covyz;
	covariancematrix(2, 2) = original.covzz;
	//- Calculate the inverse
	Eigen::Matrix3d inversecovariance = covariancematrix.inverse();

	Eigen::Vector3d mu = Eigen::Vector3d(original.mux, original.muy, original.muz);
	
	double amplitude = original.weight;
	if (normalizedWeight) {
		amplitude = amplitude / sqrt(covariancematrix.determinant()) * GAUSS_PI_FACTOR;
	}
	double pi = original.weight;
	if (!normalizedWeight) {
		pi = pi * sqrt(covariancematrix.determinant()) / GAUSS_PI_FACTOR;
	}
	return Gaussian(mu, covariancematrix, inversecovariance, amplitude, pi);
}

gmvis::core::Gaussian::Gaussian(Eigen::Vector3d mu, Eigen::Matrix3d covariancematrix, Eigen::Matrix3d inversecovariance, double amplitude, double pi)
	: m_mu(mu), m_covariancematrix(covariancematrix), m_inversecovariance(inversecovariance), m_amplitude(amplitude)
{
	//---- Create the GPU Data ----
	//1. Store inverse covariance matrix as float array to convert to QMatrix4x4. GPU uses float values.
	float covdata[16] = {
		(float)inversecovariance(0,0), (float)inversecovariance(0,1), (float)inversecovariance(0,2), 0,
		(float)inversecovariance(1,0), (float)inversecovariance(1,1), (float)inversecovariance(1,2), 0,
		(float)inversecovariance(2,0), (float)inversecovariance(2,1), (float)inversecovariance(2,2), 0,
		0.0f, 0.0f, 0.0f, 1.0f };
	//2. Create GPU Data object
	m_gpudata = { QVector4D((float)mu.x(), (float)mu.y(), (float)mu.z(), float(amplitude)), QMatrix4x4(covdata) };
	m_pi = pi;

	//---- Calculate Eigenmatrix ----
	Eigen::EigenSolver<Eigen::Matrix3Xd> eigensolver;
	eigensolver.compute(covariancematrix, true);
	Eigen::VectorXd eigen_values = eigensolver.eigenvalues().real();
	Eigen::MatrixXd eigen_vectors = eigensolver.eigenvectors().real();
	double l0 = sqrt(eigen_values(0));
	double l1 = sqrt(eigen_values(1));
	double l2 = sqrt(eigen_values(2));
	double values[9] = {
		l0 * eigen_vectors(0, 0), l1 * eigen_vectors(0, 1), l2 * eigen_vectors(0, 2),
		l0 * eigen_vectors(1, 0), l1 * eigen_vectors(1, 1), l2 * eigen_vectors(1, 2),
		l0 * eigen_vectors(2, 0), l1 * eigen_vectors(2, 1), l2 * eigen_vectors(2, 2) };

	m_eigenmatrix = QGenericMatrix<3, 3, double>(values);
}

double Gaussian::sample(double x, double y, double z) const {
	Eigen::Vector3d relpos = Eigen::Vector3d(x, y, z) - m_mu;
	double ex = std::exp(-0.5 * (relpos.transpose() * m_inversecovariance * relpos).x());
	float val = m_amplitude * ex;
	return val;
}

const QGenericMatrix<3, 3, double>& Gaussian::getEigenMatrix() const {
	return m_eigenmatrix;
}

const Eigen::Vector3d& gmvis::core::Gaussian::getPosition() const
{
	return m_mu;
}

const double& gmvis::core::Gaussian::getAmplitude() const
{
	return m_amplitude;
}

const double& gmvis::core::Gaussian::getNormalizedWeight() const
{
	return m_pi;
}

double gmvis::core::Gaussian::getCovDeterminant() const
{
	return m_covariancematrix.determinant();
}

std::optional<QMatrix4x4> Gaussian::getTransform(double threshold) const {
	if (threshold >= m_amplitude) {
		return {};
	}
	double scalar = sqrt(-2 * log(threshold / m_amplitude));
	QGenericMatrix<3, 3, double> mat = m_eigenmatrix * scalar;
	QMatrix4x4 mat4 = QMatrix4x4(
		(float)mat(0, 0), (float)mat(0, 1), (float)mat(0, 2), (float)m_mu.x(),
		(float)mat(1, 0), (float)mat(1, 1), (float)mat(1, 2), (float)m_mu.y(),
		(float)mat(2, 0), (float)mat(2, 1), (float)mat(2, 2), (float)m_mu.z(),
		0.0f, 0.0f, 0.0f, 1.0f
	);
	if (mat4.determinant() < 0) {
		mat4 = mat4 * QMatrix4x4(-1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1);
	}
	return mat4;
}

bool Gaussian::getBoundingBox(double threshold, QVector3D& min, QVector3D& max) const {
	//This ellipsoid only exists, if the threshold is smaller than the amplitude. Otherwise no result will be given
	if (threshold >= m_amplitude) {
		return false;
	}
	double scalar = sqrt(-2 * log(threshold / m_amplitude));
	QGenericMatrix<3, 3, double> transfo = m_eigenmatrix * scalar;
	QVector3D r0 = QVector3D((float)transfo(0, 0), (float)transfo(0, 1), (float)transfo(0, 2));
	QVector3D r1 = QVector3D((float)transfo(1, 0), (float)transfo(1, 1), (float)transfo(1, 2));
	QVector3D r2 = QVector3D((float)transfo(2, 0), (float)transfo(2, 1), (float)transfo(2, 2));
	QVector3D delta = QVector3D(r0.length(), r1.length(), r2.length());
	QVector3D muq = QVector3D((float)m_mu.x(), (float)m_mu.y(), (float)m_mu.z());
	min = muq - delta;
	max = muq + delta;
	return true;
}

bool gmvis::core::Gaussian::checkValidity() const
{
	bool ok = m_covariancematrix.determinant() > 0;
	if (!ok) 
		return false;
	ok = m_covariancematrix.block(0, 0, 2, 2).determinant() > 0;
	if (!ok) 
		return false;
	ok = m_covariancematrix(0, 0) > 0;
	if (!ok) 
		return false;
	ok = m_inversecovariance.determinant() > 0;
	if (!ok) 
		return false;
	ok = m_inversecovariance.block(0, 0, 2, 2).determinant() > 0;
	if (!ok) 
		return false;
	ok = m_inversecovariance(0, 0) > 0;
	if (!ok) 
		return false;
	ok = m_covariancematrix.isApprox(m_covariancematrix.transpose());
	if (!ok) 
		return false;
	ok = m_inversecovariance.isApprox(m_inversecovariance.transpose());
	if (!ok) 
		return false;
	return true;
}

const GaussianGPU& Gaussian::getGPUData() const
{
	return m_gpudata;
}