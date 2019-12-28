#pragma once
#include "qcolor.h"

class DisplaySettings {
public:
	QColor backgroundColor = QColor(0, 0, 0);
	QColor pointcloudColor = QColor(255, 255, 255);
	float  pointSize = 1.0f;
	bool   circles = false;
};