#include "Gaussian.h"
#include <random>

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
	m_pi = pi;
	//2. Check Validty
	m_valid = checkValidity();
	//3. Create GPU Data object
	m_gpudata = { m_valid, (m_pi != 0), QVector4D((float)mu.x(), (float)mu.y(), (float)mu.z(), float(amplitude)), QMatrix4x4(covdata) };

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

template <typename decimal>
decimal Gaussian<decimal>::sample(decimal x, decimal y, decimal z) const {
	EGVector relpos = EGVector(x, y, z) - m_mu;
	decimal ex = std::exp(-0.5 * (relpos.transpose() * m_inversecovariance * relpos).x());
	decimal val = m_amplitude * ex;
	return val;
}

template <typename decimal>
const QGenericMatrix<3, 3, decimal>& Gaussian<decimal>::getEigenMatrix() const {
	return m_eigenmatrix;
}

template <typename decimal>
const EGVector& gmvis::core::Gaussian<decimal>::getPosition() const
{
	return m_mu;
}

template<typename decimal>
const EGMatrix& gmvis::core::Gaussian<decimal>::getCovarianceMatrix() const
{
	return m_covariancematrix;
}

template <typename decimal>
const decimal& gmvis::core::Gaussian<decimal>::getAmplitude() const
{
	return m_amplitude;
}

template <typename decimal>
const decimal& gmvis::core::Gaussian<decimal>::getNormalizedWeight() const
{
	return m_pi;
}

template <typename decimal>
decimal gmvis::core::Gaussian<decimal>::getCovDeterminant() const
{
	return m_covariancematrix.determinant();
}

template <typename decimal>
std::optional<QMatrix4x4> Gaussian<decimal>::getTransform(decimal threshold, bool multiply_with_amplitude) const {
    if (threshold >= abs(m_amplitude) && multiply_with_amplitude) {
		return {};
	}
    decimal scalar = sqrt(-2 * log(abs(threshold / m_amplitude)));
    if (!multiply_with_amplitude)
        scalar = 1;
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

template <typename decimal>
bool Gaussian<decimal>::getBoundingBox(decimal threshold, QVector3D& min, QVector3D& max) const {
	//This ellipsoid only exists, if the threshold is smaller than the amplitude. Otherwise no result will be given
	if (threshold >= abs(m_amplitude)) {
		return false;
	}
	decimal scalar = sqrt(-2 * log(threshold / abs(m_amplitude)));
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

template<typename decimal>
bool gmvis::core::Gaussian<decimal>::getOneSigmaBoundingBox(QVector3D& min, QVector3D& max) const
{
	QVector3D r0 = QVector3D((float)m_eigenmatrix(0, 0), (float)m_eigenmatrix(0, 1), (float)m_eigenmatrix(0, 2));
	QVector3D r1 = QVector3D((float)m_eigenmatrix(1, 0), (float)m_eigenmatrix(1, 1), (float)m_eigenmatrix(1, 2));
	QVector3D r2 = QVector3D((float)m_eigenmatrix(2, 0), (float)m_eigenmatrix(2, 1), (float)m_eigenmatrix(2, 2));
	QVector3D delta = QVector3D(r0.length(), r1.length(), r2.length());
	QVector3D muq = QVector3D((float)m_mu.x(), (float)m_mu.y(), (float)m_mu.z());
	min = muq - delta;
	max = muq + delta;
	return true;
}

template<typename decimal>
bool gmvis::core::Gaussian<decimal>::isValid() const
{
	return m_valid;
}

template<typename decimal>
std::vector<EGVector> gmvis::core::Gaussian<decimal>::sampleRandom(unsigned int N) const
{
	Eigen::SelfAdjointEigenSolver<EGMatrix> eigenSolver(m_covariancematrix);
	EGMatrix transform = eigenSolver.eigenvectors() * eigenSolver.eigenvalues().cwiseSqrt().asDiagonal();
	std::mt19937 gen{ std::random_device{}() };
	std::normal_distribution<> dist;

	std::vector<EGVector> res(N);
#pragma omp parallel for
	for (int i = 0; i < N; ++i)
	{
		//res[i] = m_mu + transform * Eigen::VectorXd{ m_mu.size() }.unaryExpr([&](auto x) { return dist(gen); });
		res[i] = m_mu + transform * EGVector().unaryExpr([&](auto x) { return dist(gen); });
	}
	return res;
}

template <typename decimal>
bool gmvis::core::Gaussian<decimal>::checkValidity() const
{
	decimal det = m_covariancematrix.determinant();
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

template <typename decimal>
const GaussianGPU& Gaussian<decimal>::getGPUData() const
{
	return m_gpudata;
}

template struct gmvis::core::Gaussian<float>;
template struct gmvis::core::Gaussian<double>;
