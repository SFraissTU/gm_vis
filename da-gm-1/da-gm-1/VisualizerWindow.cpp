#include "VisualizerWindow.h"
#include <qfiledialog.h>
#include <QDoubleValidator>
#include <QComboBox>

VisualizerWindow::VisualizerWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	(void)connect(ui.loadPointcloudAction, SIGNAL(triggered()), this, SLOT(loadPointcloudAction()));
	(void)connect(ui.loadMixtureAction, SIGNAL(triggered()), this, SLOT(loadMixtureAction()));

	(void)connect(ui.cb_displayPointcloud, SIGNAL(stateChanged(int)), this, SLOT(updateSettings()));
	(void)connect(ui.cb_displayEllipsoids, SIGNAL(stateChanged(int)), this, SLOT(updateSettings()));
	(void)connect(ui.cb_displayDensity, SIGNAL(stateChanged(int)), this, SLOT(updateSettings()));

	(void)connect(ui.spin_scalemin, SIGNAL(valueChanged(double)), this, SLOT(updateSettings()));
	(void)connect(ui.spin_scalemax, SIGNAL(valueChanged(double)), this, SLOT(updateSettings()));

	(void)connect(ui.co_densrendermode, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSettings()));
	(void)connect(ui.co_ellrendermode, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSettings()));
	(void)connect(ui.spin_accthreshold, SIGNAL(valueChanged(double)), this, SLOT(updateSettings()));
	(void)connect(ui.cb_accthauto, SIGNAL(stateChanged(int)), this, SLOT(updateSettings()));
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
	settings->rebuildacc = false;
	float accthr = settings->accelerationthreshold;
	settings->displayPoints = ui.cb_displayPointcloud->isChecked();
	settings->displayEllipsoids = ui.cb_displayEllipsoids->isChecked();
	settings->displayDensity = ui.cb_displayDensity->isChecked();
	settings->densitymin = 0.01f * ui.spin_scalemin->value();
	settings->densitymax = 0.01f * ui.spin_scalemax->value();
	settings->densityRenderMode = GMDensityRenderMode(ui.co_densrendermode->currentIndex() + 1);
	settings->ellipsoidRenderMode = GMIsoellipsoidRenderMode(ui.co_ellrendermode->currentIndex() + 1);
	settings->accelerationthresholdauto = ui.cb_accthauto->isChecked();
	if (settings->accelerationthresholdauto) {
		settings->accelerationthreshold = settings->densitymax * 0.0001f;	//ToDo: Some ungood float/double thingies
		ui.spin_accthreshold->setValue(settings->accelerationthreshold * 100);
		ui.spin_accthreshold->setEnabled(false);
	}
	else {
		settings->accelerationthreshold = 0.01f * ui.spin_accthreshold->value();
		ui.spin_accthreshold->setEnabled(true);
	}
	if (GMDensityRenderer::isAccelerated(settings->densityRenderMode) && abs(settings->accelerationthreshold - accthr) > 0.000000000001) {
		settings->rebuildacc = true;
	}
	ui.openGLWidget->update();
}
