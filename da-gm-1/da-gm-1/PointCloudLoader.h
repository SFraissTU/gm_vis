#pragma once
#include "PointCloud.h"
#include <qstring.h>

class PointCloudLoader {
public:
	static std::unique_ptr<PointCloud> readPCDfromOFF(QString path, bool switchYandZ);
};