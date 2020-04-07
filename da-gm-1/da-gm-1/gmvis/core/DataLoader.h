#pragma once
#include "PointCloud.h"
#include "GaussianMixture.h"
#include <qstring.h>
#include <qfile.h>
#include <memory>

//Convert Coordinate System: (x,y,z) -> (x,z,-y)

namespace gmvis::core {

	class DataLoader {
	public:
		static std::unique_ptr<PointCloud> readPCDfromOFF(QFile& file, bool convertCoordinateSystem);
		static std::unique_ptr<PointCloud> readPCDfromOFF(const QString& path, bool convertCoordinateSystem);

		static std::unique_ptr<GaussianMixture> readGMfromPLY(QFile& file, bool isgmm, bool convertCoordinateSystem);
		static std::unique_ptr<GaussianMixture> readGMfromPLY(const QString& path, bool isgmm, bool convertCoordinateSystem);

		static QVector<QVector3D> readTransferFunction(const QString& path);
	};
}