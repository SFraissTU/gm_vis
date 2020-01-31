#include "GaussianMixture.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <Eigen/Core>
#include <QOpenGLFunctions_4_5_Core>
#include <QStack>

double GaussianMixture::sample(double x, double y, double z) const
{
	Eigen::Vector3d pos = Eigen::Vector3d(x, y, z);
	double sum = 0.0f;
	for (int i = 0; i < gaussians.size(); ++i) {
		//if (i != 4) continue;
		const Gaussian& gauss = gaussians[i];
		sum += gauss.sample(x, y, z);
	}
	return sum;
}

std::shared_ptr<char[]> GaussianMixture::gpuData(size_t& arrsize) const
{
	GLint n = (GLint)numberOfGaussians();
	arrsize = 80 * n;
	char* result = new char[arrsize];
	char* gaussmem = result;
	for (int i = 0; i < n; i++) {
		GaussianGPU gpudata = gaussians[i].gpudata;
		memcpy(gaussmem, &gpudata.mu_amplitude, 16);
		gaussmem += 16;
		memcpy(gaussmem, gpudata.invsigma.constData(), 64);
		gaussmem += 64;
	}
	return std::shared_ptr<char[]>(result);
}

std::shared_ptr<char[]> GaussianMixture::buildOctree(double threshold, QVector<GMOctreeNode>& result, GLuint& maxTraversalMemory, size_t& arrsize) const
{
	QVector3D absMin;
	QVector3D absMax;
	struct GaussianBoundingBox {
		QVector3D min;
		QVector3D max;
	};

	result.clear();
	QVector<GaussianBoundingBox> boundingBoxes;
	int n = numberOfGaussians();
	for (int i = 0; i < n; i++) {
		GaussianBoundingBox box;
		gaussians[i].getBoundingBox(threshold, box.min, box.max);
		boundingBoxes.append(box);
		if (box.min.x() < absMin.x()) absMin.setX(box.min.x());
		if (box.min.y() < absMin.y()) absMin.setY(box.min.y());
		if (box.min.z() < absMin.z()) absMin.setZ(box.min.z());
		if (box.max.x() > absMax.x()) absMax.setX(box.max.x());
		if (box.max.y() > absMax.y()) absMax.setY(box.max.y());
		if (box.max.z() > absMax.z()) absMax.setZ(box.max.z());
	}
	//ToDo: Adapt min and max such that it becomes a regular cube
	double extent = std::max(absMax.x() - absMin.x(), std::max(absMax.y() - absMin.y(), absMax.z() - absMin.z()));
	double xenlarge = (extent - absMax.x() + absMin.x()) / 2.0;
	double yenlarge = (extent - absMax.y() + absMin.y()) / 2.0;
	double zenlarge = (extent - absMax.z() + absMin.z()) / 2.0;
	absMin = QVector3D(absMin.x() - xenlarge, absMin.y() - yenlarge, absMin.z() - zenlarge);
	absMax = QVector3D(absMax.x() + xenlarge, absMax.y() + yenlarge, absMax.z() + zenlarge);

	int maxlevels = 5;
	QStack<int> stack;	//References to result per indices
	//While building, we use the gaussianend variable as level
	result.append({ absMin, absMax, 255, -1, -1, 0 });
	stack.push(0);
	//First, we create a complete Octree, then we assign the Gaussians, then we remove empty nodes.
	while (!stack.isEmpty()) {
		int index = stack.pop();
		GMOctreeNode& node = result[index];
		int firstchildindex = result.size();
		QVector4D halfsize = (node.max - node.min) / 2.0;
		double minx = node.min.x();
		double miny = node.min.y();
		double minz = node.min.z();
		double maxx = node.max.x();
		double maxy = node.max.y();
		double maxz = node.max.z();
		double middlex = minx + halfsize.x();
		double middley = miny + halfsize.y();
		double middlez = minz + halfsize.z();
		int nextlevel = node.gaussianend + 1;
		if (nextlevel < maxlevels) {
			node.childrenstart = firstchildindex;	//Needs to happen first. Otherwise through reallocation of memory, the reference might not be up to date anymore
			result.append({ node.min, QVector4D(middlex,middley,middlez,0), 255, -1, -1, nextlevel });
			result.append({ QVector4D(middlex, miny, minz, 0), QVector4D(maxx, middley, middlez,0), 255, -1, -1, nextlevel });
			result.append({ QVector4D(middlex, middley, minz, 0), QVector4D(maxx, maxy, middlez,0), 255, -1, -1, nextlevel });
			result.append({ QVector4D(minx, middley, minz, 0), QVector4D(middlex, maxy, middlez,0), 255, -1, -1, nextlevel });
			result.append({ QVector4D(minx, miny, middlez, 0), QVector4D(middlex, middley, maxz,0), 255, -1, -1, nextlevel });
			result.append({ QVector4D(middlex, miny, middlez, 0), QVector4D(maxx, middley, maxz,0), 255, -1, -1, nextlevel });
			result.append({ QVector4D(middlex, middley, middlez, 0), QVector4D(maxx, maxy, maxz,0), 255, -1, -1, nextlevel });
			result.append({ QVector4D(minx, middley, middlez, 0), QVector4D(middlex, maxy, maxz,0), 255, -1, -1, nextlevel });
			for (int i = 7; i >= 0; --i) {
				stack.push(firstchildindex + i);
			}
		}
		else {
			node.childrenbits = 0;
		}
	}
	//Assign the Gaussians..
	QVector<QVector<int>> gaussianspernode;
	gaussianspernode.resize(result.size());
	//we use the gaussianstart-flag here as a indicator whether this node contains non-empty children or gaussians (1) or not (-1)
	//in order to delete unused ones later.
	for (int i = 0; i < n; ++i) {
		GaussianBoundingBox& box = boundingBoxes[i];
		//Find corresponding octree node. Biggest node that encloses Gaussian completely
		int currentnode = 0;
		do {
			GMOctreeNode& node = result[currentnode];
			node.gaussianstart = 1;
			//Node contains box. Check if one of the children contains the box as well
			if (node.childrenstart == -1) {
				//No children. This is it.
				gaussianspernode[currentnode].append(i);
				currentnode = -1;
			}
			else {
				int parentindex = currentnode;
				for (int cidx = node.childrenstart; currentnode == parentindex && cidx < node.childrenstart + 8; ++cidx) {
					GMOctreeNode& child = result[cidx];
					if (child.min.x() <= box.min.x() && child.min.y() <= box.min.y() && child.min.z() <= box.min.z()
						&& child.max.x() >= box.max.x() && child.max.y() >= box.max.y() && child.max.z() >= box.max.z()) {
						currentnode = cidx;
					}
				}
				if (parentindex == currentnode) {
					//No child contains box completely
					gaussianspernode[currentnode].append(i);
					currentnode = -1;
				}
			}
		} while (currentnode != -1);
	}
	//Now we delete unused nodes and create the gaussian list in the right order
	QVector<GaussianGPU> gaussianList;
	int numberOfDeletedNodesSoFar = 0;
	for (int nodeidx = 0; nodeidx < result.size(); ++nodeidx) {
		GMOctreeNode& node = result[nodeidx];
		if (node.gaussianstart == -1) {
			//This node is unused. Can be deleted.
			result.remove(nodeidx);
			nodeidx -= 1;
			numberOfDeletedNodesSoFar += 1;
		}
		else {
			//Check Gaussians
			node.gaussianstart = -1;
			QVector<int>& currentgaussians = gaussianspernode[nodeidx]; 
			if (currentgaussians.size() != 0) {
				node.gaussianstart = gaussianList.size();
				node.gaussianend = gaussianList.size() + currentgaussians.size() - 1;
			}
			for (const int* it = currentgaussians.cbegin(); it != currentgaussians.cend(); ++it) {
				gaussianList.append(gaussians[*it].gpudata);
			}

			//Check Children
			node.childrenstart -= numberOfDeletedNodesSoFar;
			for (int childbit = 0; childbit <  8; ++childbit) {
				int childidx = node.childrenstart + childbit;
				if (result[childidx].gaussianstart == -1) {
					node.childrenbits &= ~(1 << childbit);
				}
			}
		}
	}
	//Now let's calculate the maximum amount of help memory we need.
	maxTraversalMemory = std::min(result.size(), (maxlevels - 1) * 7 - (maxlevels - 2)) * sizeof(GLint);

	GLint gaussN = gaussianList.size();
	arrsize = 80 * gaussN;
	char* gaussRes = new char[arrsize];
	char* gaussmem = gaussRes;
	for (int i = 0; i < gaussN; i++) {
		GaussianGPU gpudata = gaussianList[i];
		memcpy(gaussmem, &gpudata.mu_amplitude, 16);
		gaussmem += 16;
		memcpy(gaussmem, gpudata.invsigma.constData(), 64);
		gaussmem += 64;
	}
	return std::shared_ptr<char[]>(gaussRes);
}

void GaussianMixture::normalize()
{
	double sum = 0.0f;
	for (int i = 0; i < gaussians.size(); ++i) {
		sum += gaussians[i].weight;
	}
	double factor = 1.0 / sum;
	for (int i = 0; i < gaussians.size(); ++i) {
		gaussians[i].weight *= factor;
		gaussians[i].gpudata.mu_amplitude.setW(gaussians[i].gpudata.mu_amplitude.w() * factor);
	}
}
