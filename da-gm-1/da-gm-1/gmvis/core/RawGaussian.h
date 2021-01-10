#pragma once
#include <vector>

template <typename decimal>
struct RawGaussian {
	decimal mux;
	decimal muy;
	decimal muz;
	decimal covxx;
	decimal covxy;
	decimal covxz;
	decimal covyy;
	decimal covyz;
	decimal covzz;
	decimal weight;

	static void normalize(std::vector<RawGaussian>& list) {
		float weightsum = 0;
		for (auto it = list.cbegin(); it != list.cend(); ++it) {
			weightsum += it->weight;
		}
		for (auto it = list.begin(); it != list.end(); ++it) {
			it->weight /= weightsum;
		}
	}
};