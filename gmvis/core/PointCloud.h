#pragma once
#include <QVector>
#include <QVector3D>
#include <limits>

namespace gmvis::core {

	using point_item = QVector3D;
	using point_list = QVector<point_item>;

	class PointCloud {
	private:
		point_list pointdata;
		QVector3D min;
		QVector3D max;

	public:
		PointCloud(point_list& pointdata) {
			this->pointdata = std::move(pointdata);
			double inf = std::numeric_limits<double>::infinity();
			min = QVector3D(inf, inf, inf);
			max = QVector3D(-inf, -inf, -inf);
			for (auto it = this->pointdata.begin(); it != this->pointdata.end(); ++it) {
				if (it->x() > max.x()) {
					max.setX(it->x());
				}
				if (it->y() > max.y()) {
					max.setY(it->y());
				}
				if (it->z() > max.z()) {
					max.setZ(it->z());
				}
				if (it->x() < min.x()) {
					min.setX(it->x());
				}
				if (it->y() < min.y()) {
					min.setY(it->y());
				}
				if (it->z() < min.z()) {
					min.setZ(it->z());
				}
			}
		}

		unsigned long long getDataSize() const {
			return pointdata.size() * sizeof(point_item);
		}

		size_t getPointCount() const {
			return pointdata.size();
		}

		size_t getSinglePointSize() const {
			return sizeof(point_item);
		}

		point_item* getData() {
			return pointdata.data();
		}

		QVector3D getBBMin() const {
			return min;
		}

		QVector3D getBBMax() const {
			return max;
		}
	};
}
