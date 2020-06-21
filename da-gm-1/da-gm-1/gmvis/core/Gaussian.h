#pragma once
#include "RawGaussian.h"
#include <cmath>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>
#include <Eigen/Core>
#include <Eigen/LU>
#include <Eigen/Eigenvalues>
#include <optional>

#define M_PI 3.14159265358979323846

namespace gmvis::core {

	/*
	Representation of a Gaussian that will be passed to the shader.
	*/
	struct GaussianGPU {
		/* First three components represent the center of the Gaussian.
		The fourth value is the amplitude. Necessary for shader calculations */
		QVector4D mu_alpha;
		/* The inverse covariance matrix */
		QMatrix4x4 invsigma;
	};

	/*
	Represents a single Gaussian.
	*/
	struct Gaussian {
		
	public:
		Gaussian() {};	//please don't use this

		static Gaussian createGaussian(const RawGaussian& original, bool normalizedWeight = false);
		
		const GaussianGPU& getGPUData() const;

		/*
		Samples the Gaussian at the given coordinate and returns the corresponding density value.
		*/
		double sample(double x, double y, double z) const;

		/*
		Returns a matrix containing the eigenvectors multiplied by the eigenvalues
		*/
		const QGenericMatrix<3, 3, double>& getEigenMatrix() const;

		const Eigen::Vector3d& getPosition() const;

		const double& getAmplitude() const;

		const double& getNormalizedWeight() const;

		double getCovDeterminant() const;

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
		
	private:
		/* GPU data */
		GaussianGPU m_gpudata;
		/* Covariance matrix */
		Eigen::Matrix3d m_covariancematrix;
		/* Inverse covariance matrix */
		Eigen::Matrix3d m_inversecovariance;
		/* Mean of this Gaussian */
		Eigen::Vector3d m_mu;
		/* Amplitude of this Gaussian */
		double m_amplitude;
		double m_pi;
		/* Vectors containing the eigenvectors multiplied by their eigenvalues. */
		QGenericMatrix<3, 3, double> m_eigenmatrix;

		Gaussian(Eigen::Vector3d mu, Eigen::Matrix3d covariancematrix, Eigen::Matrix3d inversecovariance, double amplitude, double beta);
	};


	const double GAUSS_PI_FACTOR = 1.0 / pow(2 * M_PI, 3.0 / 2.0);
}