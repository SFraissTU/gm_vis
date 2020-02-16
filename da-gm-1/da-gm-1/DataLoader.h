#pragma once
#include "PointCloud.h"
#include "GaussianMixture.h"
#include <qstring.h>
#include <qfile.h>

//Convert Coordinate System: (x,y,z) -> (x,z,-y)

class DataLoader {
public:
	static std::unique_ptr<PointCloud> readPCDfromOFF(QFile& file, bool convertCoordinateSystem);
	static std::unique_ptr<PointCloud> readPCDfromOFF(const QString& path, bool convertCoordinateSystem);

	static std::unique_ptr<GaussianMixture> readGMfromPLY(QFile& file, bool convertCoordinateSystem);
	static std::unique_ptr<GaussianMixture> readGMfromPLY(const QString& path, bool convertCoordinateSystem);

	static QVector<QVector3D> readTransferFunction(const QString& path);
};