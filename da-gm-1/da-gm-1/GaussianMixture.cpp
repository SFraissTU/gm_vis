#include "GaussianMixture.h"
#include <cmath>
#include <Eigen/Core>
#include <QOpenGLFunctions_4_5_Core>
#include <QStack>
#include <QTime>

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
		const GaussianGPU& gpudata = gaussians[i].getGPUData();
		memcpy(gaussmem, &gpudata.mu_beta, 16);
		gaussmem += 16;
		memcpy(gaussmem, gpudata.invsigma.constData(), 64);
		gaussmem += 64;
	}
	return std::shared_ptr<char[]>(result);
}

std::shared_ptr<char[]> GaussianMixture::gpuData(size_t& arrsize, double threshold, GLuint& numberOfComponents) const {
	GLint n = (GLint)numberOfGaussians();
	numberOfComponents = 0;
	for (int i = 0; i < n; i++) {
		if (gaussians[i].getAmplitude() > threshold) {
			numberOfComponents++;
		}
	}
	arrsize = 80 * numberOfComponents;
	char* result = new char[arrsize];
	char* gaussmem = result;
	for (int i = 0; i < n; i++) {
		const GaussianGPU& gpudata = gaussians[i].getGPUData();
		if (gaussians[i].getAmplitude() > threshold) {
			memcpy(gaussmem, &gpudata.mu_beta, 16);
			gaussmem += 16;
			memcpy(gaussmem, gpudata.invsigma.constData(), 64);
			gaussmem += 64;
		}
	}
	return std::shared_ptr<char[]>(result);
}

std::shared_ptr<char[]> GaussianMixture::buildOctree(double threshold, QVector<GMOctreeNode>& result, size_t& arrsize) const
{
	QTime time;
	time.start();
	QVector3D absMin;
	QVector3D absMax;
	struct GaussianBoundingBox {
		QVector3D min;
		QVector3D max;
		int gaussindex;
	};

	QVector<GMOctreeNode> octreelist;
	QVector<GaussianBoundingBox> boundingBoxes;
	int n = numberOfGaussians();
	for (int i = 0; i < n; i++) {
		GaussianBoundingBox box;
		if (gaussians[i].getBoundingBox(threshold, box.min, box.max)) {
			box.gaussindex = i;
			boundingBoxes.append(box);
			if (box.min.x() < absMin.x()) absMin.setX(box.min.x());
			if (box.min.y() < absMin.y()) absMin.setY(box.min.y());
			if (box.min.z() < absMin.z()) absMin.setZ(box.min.z());
			if (box.max.x() > absMax.x()) absMax.setX(box.max.x());
			if (box.max.y() > absMax.y()) absMax.setY(box.max.y());
			if (box.max.z() > absMax.z()) absMax.setZ(box.max.z());
		}
	}
	//Adapt min and max such that it becomes a regular cube
	double extent = std::max(absMax.x() - absMin.x(), std::max(absMax.y() - absMin.y(), absMax.z() - absMin.z()));
	double xenlarge = (extent - absMax.x() + absMin.x()) / 2.0;
	double yenlarge = (extent - absMax.y() + absMin.y()) / 2.0;
	double zenlarge = (extent - absMax.z() + absMin.z()) / 2.0;
	absMin = QVector3D(absMin.x() - xenlarge, absMin.y() - yenlarge, absMin.z() - zenlarge);
	absMax = QVector3D(absMax.x() + xenlarge, absMax.y() + yenlarge, absMax.z() + zenlarge);

	int maxlevels = 5;
	QStack<int> stack;	//References to octreelist per indices
	//While building, we use the gaussianend variable as level
	octreelist.append({ absMin, absMax, 255, -1, -1, 0 });
	stack.push(0);
	//First, we create a complete Octree, then we assign the Gaussians, then we remove empty nodes.
	while (!stack.isEmpty()) {
		int index = stack.pop();
		GMOctreeNode& node = octreelist[index];
		int firstchildindex = octreelist.size();
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
			octreelist.append({ node.min, QVector4D(middlex,middley,middlez,0), 255, -1, -1, nextlevel });
			octreelist.append({ QVector4D(middlex, miny, minz, 0), QVector4D(maxx, middley, middlez,0), 255, -1, -1, nextlevel });
			octreelist.append({ QVector4D(middlex, middley, minz, 0), QVector4D(maxx, maxy, middlez,0), 255, -1, -1, nextlevel });
			octreelist.append({ QVector4D(minx, middley, minz, 0), QVector4D(middlex, maxy, middlez,0), 255, -1, -1, nextlevel });
			octreelist.append({ QVector4D(minx, miny, middlez, 0), QVector4D(middlex, middley, maxz,0), 255, -1, -1, nextlevel });
			octreelist.append({ QVector4D(middlex, miny, middlez, 0), QVector4D(maxx, middley, maxz,0), 255, -1, -1, nextlevel });
			octreelist.append({ QVector4D(middlex, middley, middlez, 0), QVector4D(maxx, maxy, maxz,0), 255, -1, -1, nextlevel });
			octreelist.append({ QVector4D(minx, middley, middlez, 0), QVector4D(middlex, maxy, maxz,0), 255, -1, -1, nextlevel });
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
	gaussianspernode.resize(octreelist.size());
	//we use the gaussianstart-flag here as a indicator whether this node contains non-empty children or gaussians (1) or not (-1)
	//in order to delete unused ones later.
	int m = boundingBoxes.size();
	for (int i = 0; i < m; ++i) {
		GaussianBoundingBox& box = boundingBoxes[i];
		//Find corresponding octree node. Biggest node that encloses Gaussian completely
		int currentnode = 0;
		do {
			GMOctreeNode& node = octreelist[currentnode];
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
					GMOctreeNode& child = octreelist[cidx];
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
	//Just store the new indizes after deletion (needs to be precomputed before deletion)
	QVector<int> newIndizes = QVector<int>();
	newIndizes.resize(octreelist.size());
	newIndizes[0] = 0;
	int deletedSoFar = 0;
	int survivingNodes = 0;
	for (int i = 1; i < octreelist.size(); i++) {
		if (octreelist[i].gaussianstart == -1) {
			deletedSoFar++;
			newIndizes[i] = -1;
		}
		else {
			survivingNodes++;
			newIndizes[i] = i - deletedSoFar;
		}
	}
	//Now we assign correct child indizes and create the gaussian list in the right order
	QVector<GaussianGPU> gaussianList;
	for (int nodeidx = 0; nodeidx < octreelist.size(); ++nodeidx) {
		GMOctreeNode& node = octreelist[nodeidx];
		if (node.gaussianstart != -1) {	//if node won't be deleted
			//Check Gaussians
			node.gaussianstart = -1;
			QVector<int>& currentgaussians = gaussianspernode[nodeidx]; //access with original index
			if (currentgaussians.size() != 0) {
				node.gaussianstart = gaussianList.size();
				node.gaussianend = gaussianList.size() + currentgaussians.size() - 1;
			}
			for (const int* it = currentgaussians.cbegin(); it != currentgaussians.cend(); ++it) {
				gaussianList.append(gaussians[boundingBoxes[*it].gaussindex].getGPUData());
			}

			//Check Children
			if (node.childrenbits != 0) {
				int firstchild = -1;
				for (int childbit = 0; childbit < 8; ++childbit) {
					int childidx = node.childrenstart + childbit;
					if (octreelist[childidx].gaussianstart == -1) {
						node.childrenbits &= ~(1 << (7-childbit));
					}
					else if (firstchild == -1) {
						firstchild = childidx;
					}
				}
				if (firstchild == -1) {
					node.childrenstart = -1;
				}
				else {
					node.childrenstart = newIndizes[firstchild];
				}
			}
		}
	}
	//now we delete unused nodes
	//For effiency reasons instead of removing we just create a new result list
	result.clear();
	result.reserve(survivingNodes);
	for (int nodeidx = 0; nodeidx < octreelist.size(); ++nodeidx) {
		GMOctreeNode& node = octreelist[nodeidx];
		if (newIndizes[nodeidx] != -1) {
			result.push_back(node);
		}
	}


	GLint gaussN = gaussianList.size();
	arrsize = 80 * gaussN;
	char* gaussRes = new char[arrsize];
	char* gaussmem = gaussRes;
	for (int i = 0; i < gaussN; i++) {
		GaussianGPU gpudata = gaussianList[i];
		memcpy(gaussmem, &gpudata.mu_beta, 16);
		gaussmem += 16;
		memcpy(gaussmem, gpudata.invsigma.constData(), 64);
		gaussmem += 64;
	}
	qDebug() << "Time for building the octree: " << time.elapsed() << "ms (" << n << " Gaussians)";

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
		gaussians[i].updateWeight(gaussians[i].weight * factor);
	}
}
