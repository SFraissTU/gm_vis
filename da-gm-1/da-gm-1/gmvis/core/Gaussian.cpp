#include "Gaussian.h"
using namespace gmvis::core;

template <typename decimal>
Gaussian<decimal> gmvis::core::Gaussian<decimal>::createGaussian(const RawGaussian<decimal>& original, bool normalizedWeight)
{
	//---- Calculation of the inverse covariance matrix ----
	//- Create an Eigen object for the normal covariance matrix
	EGMatrix covariancematrix;
	covariancematrix(0, 0) = original.covxx;
	covariancematrix(0, 1) = covariancematrix(1, 0) = original.covxy;
	covariancematrix(0, 2) = covariancematrix(2, 0) = original.covxz;
	covariancematrix(1, 1) = original.covyy;
	covariancematrix(1, 2) = covariancematrix(2, 1) = original.covyz;
	covariancematrix(2, 2) = original.covzz;
	//- Calculate the inverse
	EGMatrix inversecovariance = covariancematrix.inverse();

	EGVector mu = EGVector(original.mux, original.muy, original.muz);
	
	decimal amplitude = original.weight;
	if (normalizedWeight) {
		amplitude = amplitude / sqrt(covariancematrix.determinant()) * GAUSS_PI_FACTOR;
	}
	decimal pi = original.weight;
	if (!normalizedWeight) {
		pi = pi * sqrt(covariancematrix.determinant()) / GAUSS_PI_FACTOR;
	}
	return Gaussian(mu, covariancematrix, inversecovariance, amplitude, pi);
}

template Gaussian<float> gmvis::core::Gaussian<float>::createGaussian(const RawGaussian<float>& original, bool normalizedWeight);
template Gaussian<double> gmvis::core::Gaussian<double>::createGaussian(const RawGaussian<double>& original, bool normalizedWeight);

template <typename decimal>
gmvis::core::Gaussian<decimal>::Gaussian(EGVector mu, EGMatrix covariancematrix, EGMatrix inversecovariance, decimal amplitude, decimal pi)
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
	Eigen::EigenSolver<EGMatrixX> eigensolver;
	eigensolver.compute(covariancematrix, true);
	EGVectorX eigen_values = eigensolver.eigenvalues().real();
	EGMatrixX eigen_vectors = eigensolver.eigenvectors().real();
	decimal l0 = sqrt(eigen_values(0));
	decimal l1 = sqrt(eigen_values(1));
	decimal l2 = sqrt(eigen_values(2));
	decimal values[9] = {
		l0 * eigen_vectors(0, 0), l1 * eigen_vectors(0, 1), l2 * eigen_vectors(0, 2),
		l0 * eigen_vectors(1, 0), l1 * eigen_vectors(1, 1), l2 * eigen_vectors(1, 2),
		l0 * eigen_vectors(2, 0), l1 * eigen_vectors(2, 1), l2 * eigen_vectors(2, 2) };

	m_eigenmatrix = QGenericMatrix<3, 3, decimal>(values);
}

template Gaussian<float>::Gaussian(Eigen::Vector3f mu, Eigen::Matrix3f covariancematrix, Eigen::Matrix3f inversecovariance, float amplitude, float pi);
template Gaussian<double>::Gaussian(Eigen::Vector3d mu, Eigen::Matrix3d covariancematrix, Eigen::Matrix3d inversecovariance, double amplitude, double pi);

template <typename decimal>
decimal Gaussian<decimal>::sample(decimal x, decimal y, decimal z) const {
	EGVector relpos = EGVector(x, y, z) - m_mu;
	decimal ex = std::exp(-0.5 * (relpos.transpose() * m_inversecovariance * relpos).x());
	decimal val = m_amplitude * ex;
	return val;
}

template float Gaussian<float>::sample(float x, float y, float z) const;
template double Gaussian<double>::sample(double x, double y, double z) const;

template <typename decimal>
const QGenericMatrix<3, 3, decimal>& Gaussian<decimal>::getEigenMatrix() const {
	return m_eigenmatrix;
}

template const QGenericMatrix<3, 3, float>& Gaussian<float>::getEigenMatrix() const;
template const QGenericMatrix<3, 3, double>& Gaussian<double>::getEigenMatrix() const;

template <typename decimal>
const EGVector& gmvis::core::Gaussian<decimal>::getPosition() const
{
	return m_mu;
}

template const Eigen::Vector3f& Gaussian<float>::getPosition() const;
template const Eigen::Vector3d& Gaussian<double>::getPosition() const;

template <typename decimal>
const decimal& gmvis::core::Gaussian<decimal>::getAmplitude() const
{
	return m_amplitude;
}

template const float& Gaussian<float>::getAmplitude() const;
template const double& Gaussian<double>::getAmplitude() const;

template <typename decimal>
const decimal& gmvis::core::Gaussian<decimal>::getNormalizedWeight() const
{
	return m_pi;
}

template const float& Gaussian<float>::getNormalizedWeight() const;
template const double& Gaussian<double>::getNormalizedWeight() const;

template <typename decimal>
decimal gmvis::core::Gaussian<decimal>::getCovDeterminant() const
{
	return m_covariancematrix.determinant();
}

template float Gaussian<float>::getCovDeterminant() const;
template double Gaussian<double>::getCovDeterminant() const;

template <typename decimal>
std::optional<QMatrix4x4> Gaussian<decimal>::getTransform(decimal threshold) const {
	if (threshold >= m_amplitude) {
		return {};
	}
	decimal scalar = sqrt(-2 * log(threshold / m_amplitude));
	QGenericMatrix<3, 3, decimal> mat = m_eigenmatrix * scalar;
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

template std::optional<QMatrix4x4> Gaussian<float>::getTransform(float threshold) const;
template std::optional<QMatrix4x4> Gaussian<double>::getTransform(double threshold) const;

template <typename decimal>
bool Gaussian<decimal>::getBoundingBox(decimal threshold, QVector3D& min, QVector3D& max) const {
	//This ellipsoid only exists, if the threshold is smaller than the amplitude. Otherwise no result will be given
	if (threshold >= m_amplitude) {
		return false;
	}
	decimal scalar = sqrt(-2 * log(threshold / m_amplitude));
	QGenericMatrix<3, 3, decimal> transfo = m_eigenmatrix * scalar;
	QVector3D r0 = QVector3D((float)transfo(0, 0), (float)transfo(0, 1), (float)transfo(0, 2));
	QVector3D r1 = QVector3D((float)transfo(1, 0), (float)transfo(1, 1), (float)transfo(1, 2));
	QVector3D r2 = QVector3D((float)transfo(2, 0), (float)transfo(2, 1), (float)transfo(2, 2));
	QVector3D delta = QVector3D(r0.length(), r1.length(), r2.length());
	QVector3D muq = QVector3D((float)m_mu.x(), (float)m_mu.y(), (float)m_mu.z());
	min = muq - delta;
	max = muq + delta;
	return true;
}

template bool Gaussian<float>::getBoundingBox(float threshold, QVector3D& min, QVector3D& max) const;
template bool Gaussian<double>::getBoundingBox(double threshold, QVector3D& min, QVector3D& max) const;

template <typename decimal>
bool gmvis::core::Gaussian<decimal>::checkValidity() const
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

template bool Gaussian<float>::checkValidity() const;
template bool Gaussian<double>::checkValidity() const;

template <typename decimal>
const GaussianGPU& Gaussian<decimal>::getGPUData() const
{
	return m_gpudata;
}

template const GaussianGPU& Gaussian<float>::getGPUData() const;
template const GaussianGPU& Gaussian<double>::getGPUData() const;