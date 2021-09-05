#include "Sampler.h"

using namespace gmvis::core;

std::vector<std::vector<std::vector<DECIMAL_TYPE>>> Sampler::sample(GaussianMixture<DECIMAL_TYPE>* mix)
{
	QVector3D min, max;
	mix->computeEllipsoidsBoundingBox(min, max);
	QVector3D delta = (max - min) / 100;
	std::vector<std::vector<std::vector<DECIMAL_TYPE>>> data = std::vector<std::vector<std::vector<DECIMAL_TYPE>>>(100, std::vector<std::vector<DECIMAL_TYPE>>(100, std::vector<DECIMAL_TYPE>(100, 0)));
	for (int x = 0; x < 100; x++)
	{
		for (int y = 0; y < 100; y++)
		{
			for (int z = 0; z < 100; z++)
			{
				QVector3D point(min.x() + delta.x() * x, min.y() + delta.y() * y, min.z() + delta.z() * z);
				float sample = mix->sample(point.x(), point.y(), point.z());
				data[x][y][z] = sample;
			}
		}
	}
	return data;
}

std::vector<std::vector<std::vector<DECIMAL_TYPE>>> Sampler::sampleCellIntegral(GaussianMixture<DECIMAL_TYPE>* mix)
{
	QVector3D min, max;
	mix->computeEllipsoidsBoundingBox(min, max);
	int res = 100;// 1000;
	QVector3D delta = (max - min) / res;
	std::vector<std::vector<std::vector<DECIMAL_TYPE>>> data = std::vector<std::vector<std::vector<DECIMAL_TYPE>>>(res, std::vector<std::vector<DECIMAL_TYPE>>(res, std::vector<DECIMAL_TYPE>(res, 0)));
	unsigned int samplecount = 1000;// 000;
	using decimal = DECIMAL_TYPE;
	for (int i = 0; i < mix->numberOfGaussians(); ++i)
	{
		std::cout << i << "/" << mix->numberOfGaussians() << std::endl;
		auto gaussian = (*mix)[i];
		auto randsamples = gaussian->sampleRandom(samplecount);
		for (int j = 0; j < samplecount; ++j)
		{
			EGVector samp = randsamples[j];
			//std::cout << samp << std::endl << "--" << std::endl;
			int xrel = res * (samp.x() - min.x()) / (max.x() - min.x());
			int yrel = res * (samp.y() - min.y()) / (max.y() - min.y());
			int zrel = res * (samp.z() - min.z()) / (max.z() - min.z());
			if (xrel >= 0 && xrel < res && yrel >= 0 && yrel < res && zrel >= 0 && zrel < res)
			{
				data[xrel][yrel][zrel] += gaussian->getNormalizedWeight() / samplecount;
			}
		}
	}

	return data;
}

std::vector<std::vector<std::vector<float>>> gmvis::core::Sampler::cellIntegralFromPointcloud(PointCloud* pc)
{
	QVector3D min = pc->getBBMin();
	QVector3D max = pc->getBBMax();
	int res = 100;
	QVector3D delta = (max - min) / res;
	std::vector<std::vector<std::vector<DECIMAL_TYPE>>> data = std::vector<std::vector<std::vector<DECIMAL_TYPE>>>(res, std::vector<std::vector<DECIMAL_TYPE>>(res, std::vector<DECIMAL_TYPE>(res, 0)));
	using decimal = DECIMAL_TYPE;

	for (int j = 0; j < pc->getNumberOfPoints(); ++j)
	{
		QVector3D samp = pc->getPoint(j);
		int xrel = res * (samp.x() - min.x()) / (max.x() - min.x());
		int yrel = res * (samp.y() - min.y()) / (max.y() - min.y());
		int zrel = res * (samp.z() - min.z()) / (max.z() - min.z());
		//std::cout << samp.x() << ";" << samp.y() << ";" << samp.z() << ";" << xrel << ";" << yrel << ";" << zrel << std::endl;
		//std::cout << min.x() + xrel * delta.x() << " - " << min.x() + (xrel + 1) * delta.x() << std::endl;
		//std::cout << min.y() + yrel * delta.y() << " - " << min.y() + (yrel + 1) * delta.y() << std::endl;
		//std::cout << min.z() + zrel * delta.z() << " - " << min.z() + (zrel + 1) * delta.z() << std::endl;
		if (xrel >= 0 && xrel < res && yrel >= 0 && yrel < res && zrel >= 0 && zrel < res)
		{
			data[xrel][yrel][zrel] += 1.0 / pc->getNumberOfPoints();
		}
	}

	return data;
}

void gmvis::core::Sampler::writeToDat(const std::vector<std::vector<std::vector<float>>>& data, FILE& out, float max)
{
	unsigned short width = data.size();
	unsigned short height = data[0].size();
	unsigned short depth = data[0][0].size();
	fwrite(&width, sizeof(unsigned short), 1, &out);
	fwrite(&height, sizeof(unsigned short), 1, &out);
	fwrite(&depth, sizeof(unsigned short), 1, &out);
	std::vector<unsigned short> fdata;
	fdata.resize(width * height * depth);
	for (int z = 0; z < depth; ++z)
	{
		std::cout << "Z: " << z << std::endl;
#pragma omp parallel for
		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				//if (data[x][height - 1 - y][depth - 1 - z] != 0)
					//std::cout << data[x][height - 1 - y][depth - 1 - z] << "-->" << std::min(data[x][height - 1 - y][depth - 1 - z] / max, 1.0f) * 255 << std::endl;
				fdata[x + y * width + z * width * height] = std::min(data[x][height-1-y][depth-1-z] / max, 1.0f) * 255;
				//unsigned short val = data[x][height - 1 - y][depth - 1 - z] / max * 255;
				//fwrite(&val, sizeof(unsigned short), 1, &out);
			}
		}
	}
	fwrite((void*)&(fdata.front()), sizeof(unsigned short), width * height * depth, &out);
	fclose(&out);
}
