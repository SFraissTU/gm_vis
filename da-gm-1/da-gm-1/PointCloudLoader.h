#pragma once
#include "PointCloud.h"
#include <qstring.h>
#include <qfile.h>

class PointCloudLoader {
public:
	static std::unique_ptr<PointCloud> readPCDfromOFF(QFile& file, bool switchYandZ);
	static std::unique_ptr<PointCloud> readPCDfromOFF(QString path, bool switchYandZ);
};