#pragma once
#include <QColor>
#include <QVector3D>

class DisplaySettings {
public:
	QColor backgroundColor = QColor(0, 0, 0);
	QColor pointcloudColor = QColor(255, 255, 255);
	float  pointSize = 1.0f;
	bool   circles = false;

	QColor ellipsoidColor = QColor(100, 100, 255);
	QVector3D lightDirection = QVector3D(0.f, -0.7f, -1.0f).normalized();
};