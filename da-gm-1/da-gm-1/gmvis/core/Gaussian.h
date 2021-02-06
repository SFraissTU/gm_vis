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
#define EGVector Eigen::Matrix<decimal, 3, 1>
#define EGMatrix Eigen::Matrix<decimal, 3, 3>
#define EGVectorX Eigen::Matrix<decimal, -1, 1>
#define EGMatrixX Eigen::Matrix<decimal, -1, -1>

namespace gmvis::core {

	/*
	Representation of a Gaussian that will be passed to the shader.
	*/
	struct GaussianGPU {
		bool isvalid;
		bool isnonzero;
		/* First three components represent the center of the Gaussian.
		The fourth value is the amplitude. Necessary for shader calculations */
		QVector4D mu_alpha;
		/* The inverse covariance matrix */
		QMatrix4x4 invsigma;
	};

	/*
	Represents a single Gaussian.
	*/
	template <typename decimal>
	struct Gaussian {
		
	public:
		Gaussian() {};	//please don't use this

		static Gaussian createGaussian(const RawGaussian<decimal>& original, bool normalizedWeight = false);
		
		const GaussianGPU& getGPUData() const;

		/*
		Samples the Gaussian at the given coordinate and returns the corresponding density value.
		*/
		decimal sample(decimal x, decimal y, decimal z) const;

		/*
		Returns a matrix containing the eigenvectors multiplied by the eigenvalues
		*/
		const QGenericMatrix<3, 3, decimal>& getEigenMatrix() const;

		const EGVector& getPosition() const;

		const EGMatrix& getCovarianceMatrix() const;

		const decimal& getAmplitude() const;

		const decimal& getNormalizedWeight() const;

		decimal getCovDeterminant() const;

		/*
		Takes the isoellipsoid of this gaussian with the constant density value
		defined by threshold and calculates a transformation matrix that transforms
		a simple unit sphere at the origin to this ellipsoid.
		Returns this matrix as a float matrix, or returns nothing if the threshold
		does not result in an isoellipsoid, as it's bigger than any density value
		appearing in this Gaussian.
		*/
        std::optional<QMatrix4x4> getTransform(decimal threshold, bool multiply_with_amplitude = true) const;

		/*
		Takes the isoellipsoid of this gaussian with the constant density value
		defined by threshold and calculates the smallest axis aligned bounding box
		that fully contains this ellipsoid.
		If the isoellipsoid exists, true is returned and the min and max values
		of the AABB are stored in the parameters. Otherwise, false is returned.
		*/
		bool getBoundingBox(decimal threshold, QVector3D& min, QVector3D& max) const;

		bool isValid() const;
		
	private:
		/* GPU data */
		GaussianGPU m_gpudata;
		/* Covariance matrix */
		EGMatrix m_covariancematrix;
		/* Inverse covariance matrix */
		EGMatrix m_inversecovariance;
		/* Mean of this Gaussian */
		EGVector m_mu;
		/* Amplitude of this Gaussian */
		decimal m_amplitude;
		decimal m_pi;
		bool m_valid;
		/* Vectors containing the eigenvectors multiplied by their eigenvalues. */
		QGenericMatrix<3, 3, decimal> m_eigenmatrix;

		Gaussian(EGVector mu, EGMatrix covariancematrix, EGMatrix inversecovariance, decimal amplitude, decimal beta);
		bool checkValidity() const;

		static const inline decimal GAUSS_PI_FACTOR = 1.0 / pow(2 * M_PI, 3.0 / 2.0);
	};

    extern template struct gmvis::core::Gaussian<float>;
    extern template struct gmvis::core::Gaussian<double>;
}
