#include "Gaussian.h"

using namespace gmvis::core;

const GaussianGPU& Gaussian::getGPUData() const
{
	return gpudata;
}

void Gaussian::finalizeInitialization() {

	//---- Calculation of the inverse covariance matrix ----
	//- Create an Eigen object for the normal covariance matrix
	Eigen::Matrix3d covariancematrix;
	covariancematrix(0, 0) = covxx;
	covariancematrix(0, 1) = covariancematrix(1, 0) = covxy;
	covariancematrix(0, 2) = covariancematrix(2, 0) = covxz;
	covariancematrix(1, 1) = covyy;
	covariancematrix(1, 2) = covariancematrix(2, 1) = covyz;
	covariancematrix(2, 2) = covzz;
	//- Calculate the inverse
	inversecovariance = covariancematrix.inverse();

	//---- Calcualte the amplitude ----
	amplitude = weight / (sqrt(covariancematrix.determinant())) * GAUSS_PI_FACTOR;

	//---- Create the mu vector ----
	mu = Eigen::Vector3d(mux, muy, muz);

	//---- Create the GPU Data ----
	//1. Store inverse covariance matrix as float array to convert to QMatrix4x4. GPU uses float values.
	float covdata[16] = { 
		(float)inversecovariance(0,0), (float)inversecovariance(0,1), (float)inversecovariance(0,2), 0,
		(float)inversecovariance(1,0), (float)inversecovariance(1,1), (float)inversecovariance(1,2), 0,
		(float)inversecovariance(2,0), (float)inversecovariance(2,1), (float)inversecovariance(2,2), 0,
		0.0f, 0.0f, 0.0f, 1.0f };
	//2. Create GPU Data object
	gpudata = { QVector4D((float)mu.x(), (float)mu.y(), (float)mu.z(), float(SQRT_TWO_PI * amplitude)), QMatrix4x4(covdata) };

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

	eigenmatrix = QGenericMatrix<3,3,double>(values);
}

void Gaussian::updateWeight(double weight)
{
	amplitude = (amplitude / this->weight) * weight;
	this->weight = weight;
	gpudata.mu_beta.setW(float(SQRT_TWO_PI * amplitude));
}

const double& Gaussian::getAmplitude() const
{
	return amplitude;
}

double Gaussian::sample(double x, double y, double z) const {
	Eigen::Vector3d relpos = Eigen::Vector3d(x - this->mux, y - this->muy, z - this->muz);
	double ex = std::exp(-0.5 * (relpos.transpose() * inversecovariance * relpos).x());
	float val = amplitude * ex;
	return val;
}

const QGenericMatrix<3, 3, double>& Gaussian::getEigenMatrix() const {
	return eigenmatrix;
}

std::optional<QMatrix4x4> Gaussian::getTransform(double threshold) const {
	if (threshold >= amplitude) {
		return {};
	}
	double scalar = sqrt(-2 * log(threshold / amplitude));
	QGenericMatrix<3,3,double> mat = eigenmatrix * scalar;
	QMatrix4x4 mat4 = QMatrix4x4(
		(float)mat(0, 0), (float)mat(0, 1), (float)mat(0, 2), (float)mux,
		(float)mat(1, 0), (float)mat(1, 1), (float)mat(1, 2), (float)muy,
		(float)mat(2, 0), (float)mat(2, 1), (float)mat(2, 2), (float)muz,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	if (mat4.determinant() < 0) {
		mat4 = mat4 * QMatrix4x4(-1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1);
	}
	return mat4;
}

bool Gaussian::getBoundingBox(double threshold, QVector3D& min, QVector3D& max) const {
	//This ellipsoid only exists, if the threshold is smaller than the amplitude. Otherwise no result will be given
	if (threshold >= amplitude) {
		return false;
	}
	double scalar = sqrt(-2 * log(threshold / amplitude));
	QGenericMatrix<3, 3, double> transfo = eigenmatrix * scalar;
	QVector3D r0 = QVector3D((float)transfo(0, 0), (float)transfo(0, 1), (float)transfo(0, 2));
	QVector3D r1 = QVector3D((float)transfo(1, 0), (float)transfo(1, 1), (float)transfo(1, 2));
	QVector3D r2 = QVector3D((float)transfo(2, 0), (float)transfo(2, 1), (float)transfo(2, 2));
	QVector3D delta = QVector3D(r0.length(), r1.length(), r2.length());
	QVector3D muq = QVector3D((float)mu.x(), (float)mu.y(), (float)mu.z());
	min = muq - delta;
	max = muq + delta;
	return true;
}