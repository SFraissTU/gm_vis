#pragma once
#include <cmath>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>
#include <Eigen/Core>
#include <Eigen/LU>
#include <Eigen/Eigenvalues>
#include <optional>

#define M_PI 3.14159265358979323846


const double GAUSS_PI_FACTOR = 1.0 / pow(2 * M_PI, 3.0 / 2.0);

/*
Representation of a Gaussian that will be passed to the shader.
*/
struct GaussianGPU {
	/* First three components represent the center of the Gaussian. 
	The fourth value is the amplitude, so the value with which exp(...) is multiplied */
	QVector4D mu_amplitude;
	/* The inverse covariance matrix */
	QMatrix4x4 invsigma;
};

/*
Represents a single Gaussian.
*/
struct Gaussian {
	/* x coordinate of the mean */
	double mux;
	/* y coordinate of the mean */
	double muy;
	/* z coordinate of the mean */
	double muz;
	/* variance of x */
	double covxx;
	/* covariance of x and y */
	double covxy;
	/* covariance of x and z */
	double covxz;
	/* variance of y */
	double covyy;
	/* covariance of y and z */
	double covyz;
	/* variance of z */
	double covzz;
	/* weight of this gaussian (commonly referred to as pi_k) */
	double weight;

	const GaussianGPU& getGPUData() const;

private:
	/* GPU data */
	GaussianGPU gpudata;
	/* Inverse covariance matrix */
	Eigen::Matrix3d inversecovariance;
	/* Amplitude of this Gaussian, meaning what exp is multiplied with: pi_k/((2pi)^(3/2) * det(sigma)^(1/2)  */
	double amplitude;
	/* Mean of this Gaussian */
	Eigen::Vector3d mu;
	/* Vectors containing the eigenvectors multiplied by their eigenvalues. */
	QGenericMatrix<3,3,double> eigenmatrix;

public:
	/*
	To initialize a Gaussian, the public values have to be set,
	and then this function has to be called.
	It initializes the private help members.
	Other functions will not work correctly if this function has not been called.
	*/
	void finalizeInitialization();

	/*
	Samples the Gaussian at the given coordinate and returns the corresponding density value.
	*/
	double sample(double x, double y, double z) const;

	/*
	Returns a matrix containing the eigenvectors multiplied by the eigenvalues
	*/
	const QGenericMatrix<3, 3, double>& getEigenMatrix() const;

	/*
	Takes the isoellipsoid of this gaussian with the constant density value
	defined by threshold and calculates a transformation matrix that transforms
	a simple unit sphere at the origin to this ellipsoid.
	Returns this matrix as a float matrix, or returns nothing if the threshold
	does not result in an isoellipsoid, as it's bigger than any density value
	appearing in this Gaussian.
	*/
	std::optional<QMatrix4x4> getTransform(double threshold) const;

	/*
	Takes the isoellipsoid of this gaussian with the constant density value
	defined by threshold and calculates the smallest axis aligned bounding box
	that fully contains this ellipsoid.
	If the isoellipsoid exists, true is returned and the min and max values
	of the AABB are stored in the parameters. Otherwise, false is returned.
	*/
	bool getBoundingBox(double threshold, QVector3D& min, QVector3D& max) const;
};