#pragma once
#include <vector>

struct RawGaussian {
	double mux;
	double muy;
	double muz;
	double covxx;
	double covxy;
	double covxz;
	double covyy;
	double covyz;
	double covzz;
	double weight;

	static void normalize(std::vector<RawGaussian>& list) {
		double weightsum = 0;
		for (auto it = list.cbegin(); it != list.cend(); ++it) {
			weightsum += it->weight;
		}
		for (auto it = list.begin(); it != list.end(); ++it) {
			it->weight /= weightsum;
		}
	}
};