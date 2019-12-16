#include "VisualizerWindow.h"
#include <qfiledialog.h>

VisualizerWindow::VisualizerWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	connect(ui.loadPointcloudAction, SIGNAL(triggered()), this, SLOT(loadPointcloudAction()));

	pointcloud = PointCloudLoader::readPCDfromOFF("data/chair_0129.off", true);
	
	ui.openGLWidget->setPointCloud(pointcloud.get());
}

void VisualizerWindow::loadPointcloudAction() {
	QString filename = QFileDialog::getOpenFileName(this, "Load Pointcloud", QString(), "*.off");
	if (!filename.isNull()) {
		auto newPC = PointCloudLoader::readPCDfromOFF(filename, true);
		if (newPC) {
			pointcloud = std::move(newPC);
			ui.openGLWidget->setPointCloud(pointcloud.get());
		}
	}
}
