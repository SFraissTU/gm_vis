#pragma once
#include <QVector>
#include <QOpenGLFunctions_4_5_Core>
#include <memory>
#include "Gaussian.h"

namespace gmvis::core {

	/*
	Represents a single node of an acceleration octree for a Gaussian Mixture
	*/
	struct GMOctreeNode {
		/* Minimum of the AABB of this node */
		QVector4D min;
		/* Maximum of the AABB of this node */
		QVector4D max;
		/* The first 8 bits indicate which child nodes exist and which not */
		GLint childrenbits;
		/* If this node has child nodes, this is the index of the first child node. Otherwise -1 */
		GLint childrenstart;
		/* If this node has Gaussians, this is the index of the first corresponding Gaussian in the list. Otherwise -1 */
		GLint gaussianstart;
		/* If this node has Gaussians, this is the index of the last corresponding Gaussian in the list. Otherwise undefined. */
		GLint gaussianend;
	};

	template <typename decimal>
	class GaussianMixture {
	private:
		QVector<Gaussian<decimal>> gaussians;
		bool disableInvalidGaussians = false;
		bool disableZeroGaussians = false;
		bool hasNegativeGaussians = false;

	public:
		GaussianMixture() {};

		GaussianMixture(const std::vector<RawGaussian<decimal>>& gaussians, bool isgmm);

		void addGaussian(const RawGaussian<decimal>& gauss);

		void addGaussian(const Gaussian<decimal>& gauss) {
			gaussians.push_back(gauss);
			if (gauss.getNormalizedWeight() < 0 || gauss.getAmplitude() < 0) {
				hasNegativeGaussians = true;
			}
		}

		int numberOfGaussians() const {
			return gaussians.size();
		}

		void setDisableInvalidGaussians(bool disable);
		void setDisableZeroGaussians(bool disable);
		
		bool containsNegativeGaussians() const;

		decimal sample(decimal x, decimal y, decimal z) const;

		const Gaussian<decimal>* operator[](int index) const {
			if (index >= gaussians.size() || index < 0) {
				return nullptr;
			}
			else {
				return &gaussians[index];
			}
		}

		bool isValid() const;

		int nextEnabledGaussianIndex(int previous = -1) const;
		int gaussIndexFromEnabledGaussIndex(int index) const;
		int enabledGaussIndexFromGaussIndex(int index) const;

		//void computePositionsBoundingBox(QVector3D& min, QVector3D& max) const; //should not be used
		void computeEllipsoidsBoundingBox(QVector3D& min, QVector3D& max) const;

		std::shared_ptr<char[]> gpuData(size_t& arrsize, GLuint& numberOfComponents) const;
		std::shared_ptr<char[]> gpuData(size_t& arrsize, decimal threshold, GLuint& numberOfComponents) const;
		std::shared_ptr<char[]> gpuPositionData(size_t& arrsize, GLuint& numberOfComponents) const;

		//Returns the Gaussians-Data as return value and the octree nodes per parameter
		std::shared_ptr<char[]> buildOctree(decimal threshold, QVector<GMOctreeNode>& result, size_t& arrsize) const;
	};

}