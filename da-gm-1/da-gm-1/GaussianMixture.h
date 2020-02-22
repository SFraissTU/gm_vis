#pragma once
#include <QVector>
#include <QOpenGLFunctions_4_5_Core>
#include <memory>
#include "Gaussian.h"

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

class GaussianMixture {
private:
	QVector<Gaussian> gaussians;

public:

	void addGaussian(const Gaussian& gauss) {
		gaussians.push_back(gauss);
	}

	int numberOfGaussians() const {
		return gaussians.size();
	}

	double sample(double x, double y, double z) const;

	const Gaussian* operator[](int index) const {
		if (index >= gaussians.size() || index < 0) {
			return nullptr;
		}
		else {
			return &gaussians[index];
		}
	}

	std::shared_ptr<char[]> gpuData(size_t& arrsize) const;
	std::shared_ptr<char[]> gpuData(size_t& arrsize, double threshold, GLuint& numberOfComponents) const;

	//Returns the Gaussians-Data as return value and the octree nodes per parameter
	std::shared_ptr<char[]> buildOctree(double threshold, QVector<GMOctreeNode>& result, size_t& arrsize) const;

	void normalize();
};
