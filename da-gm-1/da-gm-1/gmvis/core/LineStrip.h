#pragma once
#include "PointCloud.h"

namespace gmvis::core {

	class LineStrip {
	private:
		point_list pointdata;

	public:
		LineStrip(point_list& pointdata) {
			this->pointdata = std::move(pointdata);
		}

		unsigned long long getDataSize() const {
			return pointdata.size() * sizeof(point_item);
		}

		size_t getPointCount() const {
			return pointdata.size();
		}
		
		size_t getLineCount() const {
			return pointdata.size() / 2;
		}

		size_t getSinglePointSize() const {
			return sizeof(point_item);
		}

		point_item* getData() {
			return pointdata.data();
		}
	};
}