#include "VisualizerWindow.h"
#include <qfiledialog.h>

VisualizerWindow::VisualizerWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	connect(ui.loadPointcloudAction, SIGNAL(triggered()), this, SLOT(loadPointcloudAction()));
	connect(ui.loadMixtureAction, SIGNAL(triggered()), this, SLOT(loadMixtureAction()));

	/*pointcloud = DataLoader::readPCDfromOFF("data/chair_0129.off", true);
	
	ui.openGLWidget->setPointCloud(pointcloud.get());*/
}

void VisualizerWindow::loadPointcloudAction() {
	QString filename = QFileDialog::getOpenFileName(this, "Load Pointcloud", QString(), "*.off");
	if (!filename.isNull()) {
		auto newPC = DataLoader::readPCDfromOFF(filename, false);
		if (newPC) {
			pointcloud = std::move(newPC);
			ui.openGLWidget->setPointCloud(pointcloud.get());
		}
	}
}

void VisualizerWindow::loadMixtureAction()
{
	QString filename = QFileDialog::getOpenFileName(this, "Load Mixture", QString(), "*.ply");
	if (!filename.isNull()) {
		auto newGauss = DataLoader::readGMfromPLY(filename, false);
		//TODO: Files are not correctly read
		if (newGauss) {
			//successfull... store and pass to Widget
		}
	}
}
