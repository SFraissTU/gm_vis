#pragma once
#include <QColor>
#include <QVector3D>
#include "GMRenderModes.h"

class DisplaySettings {
public:
	QColor backgroundColor = QColor(0, 0, 0);
	QColor pointcloudColor = QColor(255, 255, 255);
	float  pointSize = 1.0f;
	bool   circles = false;

	bool displayPoints = false;
	bool displayEllipsoids = false;
	bool displayDensity = true;

	QColor ellipsoidColor = QColor(100, 100, 255);
	QVector3D lightDirection = QVector3D(0.f, -0.7f, -1.0f).normalized();
	GMIsoellipsoidRenderMode ellipsoidRenderMode = GMIsoellipsoidRenderMode::COLOR_UNIFORM;
	float rendermodeblending = 0.5f;

	float densitymin = 0.0f;
	float densitymax = 0.0005f;

	GMDensityRenderMode densityRenderMode = GMDensityRenderMode::ADDITIVE;
	double accelerationthreshold = 0.0000005;
	bool accelerationthresholdauto = true;
	//Octreethreshold is determined automatically from densitymax

	bool rebuildacc = false;
};