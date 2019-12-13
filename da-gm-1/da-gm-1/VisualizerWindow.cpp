#include "VisualizerWindow.h"

VisualizerWindow::VisualizerWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	//pointcloud = PointCloudLoader::readPCDfromOFF("data/chair_0002.off", true);
	//pointcloud = PointCloudLoader::readPCDfromOFF("data/chair_0048.off", true);
	//pointcloud = PointCloudLoader::readPCDfromOFF("data/chair_0084.off", true);
	pointcloud = PointCloudLoader::readPCDfromOFF("data/chair_0129.off", true);

	ui.openGLWidget->setPointCloud(pointcloud.get());
}