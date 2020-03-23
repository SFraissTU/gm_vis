#pragma once
#include <QVector>
#include <QVector3D>

namespace gmvis::core {

	using point_item = QVector3D;
	using point_list = QVector<point_item>;

	class PointCloud {
	private:
		point_list pointdata;

	public:
		PointCloud(point_list& pointdata) {
			this->pointdata = std::move(pointdata);
		}

		unsigned long long getDataSize() {
			return pointdata.size() * sizeof(point_item);
		}

		size_t getPointCount() {
			return pointdata.size();
		}

		size_t getSinglePointSize() {
			return sizeof(point_item);
		}

		point_item* getData() {
			return pointdata.data();
		}
	};
}