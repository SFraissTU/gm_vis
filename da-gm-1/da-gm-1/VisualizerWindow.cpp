#include "VisualizerWindow.h"
#include <qfiledialog.h>
#include <QDoubleValidator>

VisualizerWindow::VisualizerWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	connect(ui.loadPointcloudAction, SIGNAL(triggered()), this, SLOT(loadPointcloudAction()));
	connect(ui.loadMixtureAction, SIGNAL(triggered()), this, SLOT(loadMixtureAction()));

	connect(ui.spin_scalemin, SIGNAL(valueChanged(double)), this, SLOT(updateSettings()));
	connect(ui.spin_scalemax, SIGNAL(valueChanged(double
	)), this, SLOT(updateSettings()));
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
		if (newGauss) {
			mixture = std::move(newGauss);
			ui.openGLWidget->setGaussianMixture(mixture.get());
		}
	}
}

void VisualizerWindow::updateSettings()
{
	auto settings = ui.openGLWidget->getSettings();
	settings->densitymin = ui.spin_scalemin->value();
	settings->densitymax = ui.spin_scalemax->value();
	ui.openGLWidget->update();
}
