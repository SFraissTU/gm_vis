#include "GaussianMixture.h"
#include <cmath>
#include <Eigen/Core>
#include <QOpenGLFunctions_4_5_Core>
#include <QStack>
#include <QTime>

using namespace gmvis::core;

template <typename decimal>
GaussianMixture<decimal>::GaussianMixture(const std::vector<RawGaussian<decimal>>& gaussians, bool isgmm)
{
	for (auto it = gaussians.cbegin(); it != gaussians.cend(); ++it) {
		addGaussian(Gaussian<decimal>::createGaussian(*it, isgmm));
	}
}

template <typename decimal>
void GaussianMixture<decimal>::addGaussian(const RawGaussian<decimal>& gauss)
{
	addGaussian(Gaussian<decimal>::createGaussian(gauss, false));
}

template<typename decimal>
void gmvis::core::GaussianMixture<decimal>::setDisableInvalidGaussians(bool disable)
{
	disableInvalidGaussians = disable;
}

template<typename decimal>
void gmvis::core::GaussianMixture<decimal>::setDisableZeroGaussians(bool disable)
{
	disableZeroGaussians = disable;
}

template<typename decimal>
bool gmvis::core::GaussianMixture<decimal>::containsNegativeGaussians() const
{
	return hasNegativeGaussians;
}

template <typename decimal>
decimal GaussianMixture<decimal>::sample(decimal x, decimal y, decimal z) const
{
	EGVector pos = EGVector(x, y, z);
	decimal sum = 0.0f;
	for (int i = 0; i < gaussians.size(); ++i) {
		//if (i != 4) continue;
		const Gaussian<decimal>& gauss = gaussians[i];
		sum += gauss.sample(x, y, z);
	}
	return sum;
}

template <typename decimal>
bool gmvis::core::GaussianMixture<decimal>::isValid() const
{
	//decimal weightsum = 0.0;
	for (int i = 0; i < gaussians.size(); ++i) {
		const Gaussian<decimal>& gauss = gaussians[i];
		if (!gauss.isValid()) {
			return false;
		}
		//weightsum += gauss.getNormalizedWeight();
	}
	/*if (abs(weightsum - 1.0) > 0.01)
	{
		return false;
	}*/
	return true;
}

template<typename decimal>
int gmvis::core::GaussianMixture<decimal>::nextEnabledGaussianIndex(int previous) const
{
	for (int i = previous + 1; i < gaussians.size(); ++i)
	{
		const GaussianGPU& gpudata = gaussians[i].getGPUData();
		if ((!disableInvalidGaussians || gpudata.isvalid) && (!disableZeroGaussians || gpudata.isnonzero)) {
			return i;
		}
	}
	return -1;
}

template<typename decimal>
int gmvis::core::GaussianMixture<decimal>::gaussIndexFromEnabledGaussIndex(int index) const
{
	if (!disableInvalidGaussians && !disableZeroGaussians) {
		return index;
	}
	int currentenabledindex = -1;
	for (int i = 0; i < gaussians.size(); ++i) {
		const GaussianGPU& gpudata = gaussians[i].getGPUData();
		if ((!disableInvalidGaussians || gpudata.isvalid) && (!disableZeroGaussians || gpudata.isnonzero)) {
			//is enabled
			currentenabledindex++;
			if (currentenabledindex == index) {
				return i;
			}
		}
	}
	return -1;
}

template<typename decimal>
int gmvis::core::GaussianMixture<decimal>::enabledGaussIndexFromGaussIndex(int index) const
{
	if (!disableInvalidGaussians && !disableZeroGaussians) {
		return index;
	}
	int currentenabledindex = -1;
	for (int i = 0; i < gaussians.size(); ++i) {
		const GaussianGPU& gpudata = gaussians[i].getGPUData();
		if ((!disableInvalidGaussians || gpudata.isvalid) && (!disableZeroGaussians || gpudata.isnonzero)) {
			//is enabled
			currentenabledindex++;
			if (i == index) {
				return currentenabledindex;
			}
		}
	}
	return -1;
}

template<typename decimal>
void gmvis::core::GaussianMixture<decimal>::computePositionsBoundingBox(QVector3D& min, QVector3D& max) const
{
	constexpr float inf = std::numeric_limits<float>::infinity();
	min = QVector3D(inf, inf, inf);
	max = QVector3D(-inf, -inf, -inf);
	for (auto it = gaussians.begin(); it != gaussians.end(); ++it)
	{
		if (it->getNormalizedWeight() > 0)
		{
			EGVector pos = it->getPosition();
			for (int i = 0; i < 3; ++i)
			{
				if (min[i] > pos[i])
				{
					min[i] = pos[i];
				}
				if (max[i] < pos[i])
				{
					max[i] = pos[i];
				}
			}
		}
	}
	QVector3D extend = max - min;
	min -= 0.1 * extend;
	max += 0.1 * extend;
}

template <typename decimal>
std::shared_ptr<char[]> GaussianMixture<decimal>::gpuData(size_t& arrsize, GLuint& numberOfComponents) const
{
	GLint n = (GLint)numberOfGaussians();
	arrsize = 80 * n;
	char* result = new char[arrsize];
	char* gaussmem = result;
	GLint filteredNumberOfGaussians = 0;
	for (int i = 0; i < n; i++) {
		const GaussianGPU& gpudata = gaussians[i].getGPUData();
		if ((!disableInvalidGaussians || gpudata.isvalid) && (!disableZeroGaussians || gpudata.isnonzero)) {
			memcpy(gaussmem, &gpudata.mu_alpha, 16);
			gaussmem += 16;
			memcpy(gaussmem, gpudata.invsigma.constData(), 64);
			gaussmem += 64;
			filteredNumberOfGaussians++;
		}
	}
	if (n != filteredNumberOfGaussians) {
		arrsize = 80 * filteredNumberOfGaussians;
		char* newresult = new char[arrsize];
		memcpy(newresult, result, arrsize);
		delete[] result;
		result = newresult;
	}
	numberOfComponents = filteredNumberOfGaussians;
	return std::shared_ptr<char[]>(result);
}

template <typename decimal>
std::shared_ptr<char[]> GaussianMixture<decimal>::gpuData(size_t& arrsize, decimal threshold, GLuint& numberOfComponents) const {
	GLint n = (GLint)numberOfGaussians();
	numberOfComponents = 0;
	for (int i = 0; i < n; i++) {
		if (abs(gaussians[i].getAmplitude()) > threshold) {
			const GaussianGPU& gpudata = gaussians[i].getGPUData();
			if ((!disableInvalidGaussians || gpudata.isvalid) && (!disableZeroGaussians || gpudata.isnonzero)) {
				numberOfComponents++;
			}
		}
	}
	arrsize = 80 * numberOfComponents;
	char* result = new char[arrsize];
	char* gaussmem = result;
	for (int i = 0; i < n; i++) {
		const GaussianGPU& gpudata = gaussians[i].getGPUData();
		if (abs(gaussians[i].getAmplitude()) > threshold) {
			if ((!disableInvalidGaussians || gpudata.isvalid) && (!disableZeroGaussians || gpudata.isnonzero)) {
				memcpy(gaussmem, &gpudata.mu_alpha, 16);
				gaussmem += 16;
				memcpy(gaussmem, gpudata.invsigma.constData(), 64);
				gaussmem += 64;
			}
		}
	}
	return std::shared_ptr<char[]>(result);
}

template <typename decimal>
std::shared_ptr<char[]> gmvis::core::GaussianMixture<decimal>::gpuPositionData(size_t& arrsize, GLuint& numberOfComponents) const
{
	GLint n = (GLint)numberOfGaussians();
	arrsize = 16 * n;
	char* result = new char[arrsize];
	char* gaussmem = result;
	float one = 1.0f;
	GLint filteredNumberOfGaussians = 0;
	for (int i = 0; i < n; i++) {
		const GaussianGPU& gpudata = gaussians[i].getGPUData();
		if ((!disableInvalidGaussians || gpudata.isvalid) && (!disableZeroGaussians || gpudata.isnonzero)) {
			memcpy(gaussmem, &gpudata.mu_alpha, 12);
			memcpy(gaussmem + 12, &one, 4);
			gaussmem += 16;
			filteredNumberOfGaussians++;
		}
	}
	if (n != filteredNumberOfGaussians) {
		arrsize = 16 * filteredNumberOfGaussians;
		char* newresult = new char[arrsize];
		memcpy(newresult, result, arrsize);
		delete[] result;
		result = newresult;
	}
	numberOfComponents = filteredNumberOfGaussians;
	return std::shared_ptr<char[]>(result);
}

template <typename decimal>
std::shared_ptr<char[]> GaussianMixture<decimal>::buildOctree(decimal threshold, QVector<GMOctreeNode>& result, size_t& arrsize) const
{
	//buildOctree does not ignore invalid or zero Gaussians!

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
	int i = nextEnabledGaussianIndex(-1);
	while (i != -1) {
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
		i = nextEnabledGaussianIndex(i);
	}
	//Adapt min and max such that it becomes a regular cube
	float extent = std::max(absMax.x() - absMin.x(), std::max(absMax.y() - absMin.y(), absMax.z() - absMin.z()));
	float xenlarge = (extent - absMax.x() + absMin.x()) / 2.0;
	float yenlarge = (extent - absMax.y() + absMin.y()) / 2.0;
	float zenlarge = (extent - absMax.z() + absMin.z()) / 2.0;
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
		float minx = node.min.x();
		float miny = node.min.y();
		float minz = node.min.z();
		float maxx = node.max.x();
		float maxy = node.max.y();
		float maxz = node.max.z();
		float middlex = minx + halfsize.x();
		float middley = miny + halfsize.y();
		float middlez = minz + halfsize.z();
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
		memcpy(gaussmem, &gpudata.mu_alpha, 16);
		gaussmem += 16;
		memcpy(gaussmem, gpudata.invsigma.constData(), 64);
		gaussmem += 64;
	}
	//qDebug() << "Time for building the octree: " << time.elapsed() << "ms (" << n << " Gaussians)";

	return std::shared_ptr<char[]>(gaussRes);
}

template class GaussianMixture<float>;
template class GaussianMixture<double>;