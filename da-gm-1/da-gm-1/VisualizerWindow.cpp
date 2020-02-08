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
	connect(ui.spin_scalemax, SIGNAL(valueChanged(double)), this, SLOT(updateSettings()));

	connect(ui.cb_accelerated, SIGNAL(stateChanged(int)), this, SLOT(updateSettings()));
	connect(ui.spin_accthreshold, SIGNAL(valueChanged(double)), this, SLOT(updateSettings()));
	connect(ui.cb_accthauto, SIGNAL(stateChanged(int)), this, SLOT(updateSettings()));
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
	settings->densitymin = ui.spin_scalemin->value();
	settings->densitymax = ui.spin_scalemax->value();
	settings->accelerate = ui.cb_accelerated->isChecked();
	settings->accelerationthresholdauto = ui.cb_accthauto->isChecked();
	if (settings->accelerationthresholdauto) {
		settings->accelerationthreshold = settings->densitymax * 0.001f;
		ui.spin_accthreshold->setValue(settings->accelerationthreshold);
		ui.spin_accthreshold->setEnabled(false);
	}
	else {
		settings->accelerationthreshold = ui.spin_accthreshold->value();
		ui.spin_accthreshold->setEnabled(true);
	}
	if (settings->accelerate && settings->accelerationthreshold != accthr) {
		settings->rebuildacc = true;
	}
	ui.openGLWidget->update();
}
