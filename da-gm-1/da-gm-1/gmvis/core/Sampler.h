#pragma once
#include "GaussianMixture.h"
#include "PointCloud.h"
#include <iostream>

namespace gmvis::core
{

	class Sampler {
	public:
		static std::vector<std::vector<std::vector<float>>> sample(GaussianMixture<DECIMAL_TYPE>* mix);
		static std::vector<std::vector<std::vector<float>>> sampleCellIntegral(GaussianMixture<DECIMAL_TYPE>* mix);
		static std::vector<std::vector<std::vector<float>>> cellIntegralFromPointcloud(PointCloud* pc);
		static void writeToDat(const std::vector<std::vector<std::vector<float>>>& data, FILE& out, float max);
	};
}