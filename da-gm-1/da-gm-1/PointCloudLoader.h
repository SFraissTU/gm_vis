#pragma once
#include <vector>
#include <qvector3d.h>
#include <qstring.h>

class PointCloudLoader {
public:
	static std::vector<QVector3D> readPCDfromOFF(QString path, bool switchYandZ);
};