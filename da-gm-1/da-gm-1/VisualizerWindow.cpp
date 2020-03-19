#include "VisualizerWindow.h"
#include <qfiledialog.h>
#include <QDoubleValidator>
#include <QComboBox>

VisualizerWindow::VisualizerWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	auto widget = ui.openGLWidget;
	ui.cb_displayPointcloud->setChecked(widget->isPointDisplayEnabled());
	ui.cb_displayEllipsoids->setChecked(widget->isEllipsoidDisplayEnabled());
	ui.cb_displayDensity->setChecked(widget->isDensityDisplayEnabled());

	auto ellrenderer = widget->getGMIsoellipsoidRenderer();
	ui.spin_ellmin->setValue(ellrenderer->getEllMin());
	ui.spin_ellmin->setEnabled(ellrenderer->getRangeMode() == GMIsoellipsoidColorRangeMode::RANGE_MANUAL);
	ui.spin_ellmax->setValue(ellrenderer->getEllMax());
	ui.spin_ellmax->setEnabled(ellrenderer->getRangeMode() == GMIsoellipsoidColorRangeMode::RANGE_MANUAL);
	ui.co_ellrangemode->setCurrentIndex((int)ellrenderer->getRangeMode() - 1);
	ui.co_ellrendermode->setCurrentIndex((int)ellrenderer->getRenderMode() - 1);

	auto densrenderer = widget->getGMDensityRenderer();
	ui.spin_scalemin->setValue(densrenderer->getDensityMin() * 100);
	ui.spin_scalemax->setValue(densrenderer->getDensityMax() * 100);
	ui.co_densrendermode->setCurrentIndex((int)densrenderer->getRenderMode() - 1);
	ui.spin_accthreshold->setValue(densrenderer->getAccelerationThreshold() * 100);
	ui.spin_accthreshold->setEnabled(!densrenderer->getAccelerationThresholdAuto());
	ui.cb_accthauto->setChecked(densrenderer->getAccelerationThresholdAuto());

	(void)connect(ui.loadPointcloudAction, SIGNAL(triggered()), this, SLOT(slotLoadPointcloud()));
	(void)connect(ui.loadMixtureAction, SIGNAL(triggered()), this, SLOT(slotLoadMixture()));

	(void)connect(ui.cb_displayPointcloud, SIGNAL(stateChanged(int)), this, SLOT(slotDisplayOptionsChanged()));
	(void)connect(ui.cb_displayEllipsoids, SIGNAL(stateChanged(int)), this, SLOT(slotDisplayOptionsChanged()));
	(void)connect(ui.cb_displayDensity, SIGNAL(stateChanged(int)), this, SLOT(slotDisplayOptionsChanged()));

	(void)connect(ui.spin_ellmin, SIGNAL(valueChanged(double)), this, SLOT(slotEllValuesChanged()));
	(void)connect(ui.spin_ellmax, SIGNAL(valueChanged(double)), this, SLOT(slotEllValuesChanged()));
	(void)connect(ui.co_ellrangemode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotEllAutoValueChanged()));

	(void)connect(ui.spin_scalemin, SIGNAL(valueChanged(double)), this, SLOT(slotDensityValuesChanged()));
	(void)connect(ui.spin_scalemax, SIGNAL(valueChanged(double)), this, SLOT(slotDensityValuesChanged()));

	(void)connect(ui.co_densrendermode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDensityRenderModeChanged()));
	(void)connect(ui.co_ellrendermode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotEllipsoidRenderModeChanged()));
	(void)connect(ui.spin_accthreshold, SIGNAL(valueChanged(double)), this, SLOT(slotAccelerationThresholdChanged()));
	(void)connect(ui.cb_accthauto, SIGNAL(stateChanged(int)), this, SLOT(slotAccelerationThreshAutoChanged()));
}

void VisualizerWindow::slotLoadPointcloud() {
	QString filename = QFileDialog::getOpenFileName(this, "Load Pointcloud", QString(), "*.off");
	if (!filename.isNull()) {
		auto newPC = DataLoader::readPCDfromOFF(filename, false);
		if (newPC) {
			pointcloud = std::move(newPC);
			ui.openGLWidget->getPointCloudRenderer()->setPointCloud(pointcloud.get());
		}
	}
}

void VisualizerWindow::slotLoadMixture()
{
	QString filename = QFileDialog::getOpenFileName(this, "Load Mixture", QString(), "*.ply");
	if (!filename.isNull()) {
		auto newGauss = DataLoader::readGMfromPLY(filename, false);
		if (newGauss) {
			mixture = std::move(newGauss);
			ui.openGLWidget->getGMDensityRenderer()->setMixture(mixture.get());
			auto isoren = ui.openGLWidget->getGMIsoellipsoidRenderer();
			isoren->setMixture(mixture.get());
			ui.spin_ellmin->setValue(isoren->getEllMin());
			ui.spin_ellmax->setValue(isoren->getEllMax());
		}
	}
}

void VisualizerWindow::slotDisplayOptionsChanged()
{
	ui.openGLWidget->setPointDisplayEnabled(ui.cb_displayPointcloud->isChecked());
	ui.openGLWidget->setEllipsoidDisplayEnabled(ui.cb_displayEllipsoids->isChecked());
	ui.openGLWidget->setDensityDisplayEnabled(ui.cb_displayDensity->isChecked());
	ui.openGLWidget->update();
}

void VisualizerWindow::slotEllValuesChanged()
{
	auto renderer = ui.openGLWidget->getGMIsoellipsoidRenderer();
	renderer->setEllMin(ui.spin_ellmin->value());
	renderer->setEllMax(ui.spin_ellmax->value());
	renderer->updateColors();
	ui.openGLWidget->update();
}

void VisualizerWindow::slotEllAutoValueChanged()
{
	auto renderer = ui.openGLWidget->getGMIsoellipsoidRenderer();
	GMIsoellipsoidColorRangeMode val = GMIsoellipsoidColorRangeMode(ui.co_ellrangemode->currentIndex() + 1);
	renderer->setRangeMode(val);
	renderer->updateColors();
	if (val != GMIsoellipsoidColorRangeMode::RANGE_MANUAL) {
		ui.spin_ellmin->setValue(renderer->getEllMin());
		ui.spin_ellmax->setValue(renderer->getEllMax());
		ui.spin_ellmin->setEnabled(false);
		ui.spin_ellmax->setEnabled(false);
	}
	else {
		ui.spin_ellmin->setEnabled(true);
		ui.spin_ellmax->setEnabled(true);
	}
	ui.openGLWidget->update();
}

void VisualizerWindow::slotEllipsoidRenderModeChanged()
{
	auto renderer = ui.openGLWidget->getGMIsoellipsoidRenderer();
	renderer->setRenderMode(GMIsoellipsoidRenderMode(ui.co_ellrendermode->currentIndex() + 1));
	renderer->updateColors();
	ui.spin_ellmin->setValue(renderer->getEllMin());
	ui.spin_ellmax->setValue(renderer->getEllMax());
	ui.openGLWidget->update();
}

void VisualizerWindow::slotDensityValuesChanged()
{
	auto renderer = ui.openGLWidget->getGMDensityRenderer();
	renderer->setDensityMin(ui.spin_scalemin->value() * 0.01);
	renderer->setDensityMax(ui.spin_scalemax->value() * 0.01);
	ui.spin_accthreshold->setValue(renderer->getAccelerationThreshold() * 100);
	ui.openGLWidget->update();
}

void VisualizerWindow::slotDensityRenderModeChanged()
{
	auto renderer = ui.openGLWidget->getGMDensityRenderer();
	renderer->setRenderMode(GMDensityRenderMode(ui.co_densrendermode->currentIndex() + 1));
	ui.openGLWidget->update();
}

void VisualizerWindow::slotAccelerationThresholdChanged()
{
	auto renderer = ui.openGLWidget->getGMDensityRenderer();
	renderer->setAccelerationThreshold(0.01f * ui.spin_accthreshold->value());
	renderer->updateAccelerationData();
	ui.openGLWidget->update();
}

void VisualizerWindow::slotAccelerationThreshAutoChanged()
{
	auto renderer = ui.openGLWidget->getGMDensityRenderer();
	bool autoth = ui.cb_accthauto->isChecked();
	renderer->setAccelerationThresholdAuto(autoth);
	ui.spin_accthreshold->setEnabled(!autoth);
	ui.spin_accthreshold->setValue(renderer->getAccelerationThreshold() * 100);
	renderer->updateAccelerationData();
	ui.openGLWidget->update();
}