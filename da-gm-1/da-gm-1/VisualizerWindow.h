#pragma once

#include "PointCloud.h"
#include "DataLoader.h"
#include <memory>
#include <QtWidgets/QMainWindow>
#include "ui_VisualizerWindow.h"

class VisualizerWindow : public QMainWindow
{
	Q_OBJECT

public:
	VisualizerWindow(QWidget *parent = Q_NULLPTR);

public slots:
	void loadPointcloudAction();
	void loadMixtureAction();

private:
	Ui::VisualizerWindowClass ui;
	std::unique_ptr<PointCloud> pointcloud;
	std::unique_ptr<GaussianMixture> mixture;
};
