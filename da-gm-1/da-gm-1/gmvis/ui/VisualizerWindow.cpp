#include "gmvis/ui/VisualizerWindow.h"
#include <qfiledialog.h>
#include <QDoubleValidator>
#include <QComboBox>
#include <QMessageBox>

using namespace gmvis::ui;
using namespace gmvis::core;

VisualizerWindow::VisualizerWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	QMap<QString, QString> config = DataLoader::readConfigFile("res/config.txt");
	openPcDirectory = config.value("default_pc_path");
	openGmDirectory = config.value("default_gm_path");

	auto widget = ui.openGLWidget;
	ui.cb_displayPointcloud->setChecked(widget->isPointDisplayEnabled());
	ui.cb_displayEllipsoids->setChecked(widget->isEllipsoidDisplayEnabled());
	ui.cb_displayGMPositions->setChecked(widget->isGMPositionsDisplayEnabled());
	ui.cb_displayIsosurface->setChecked(widget->isIsosurfaceDisplayEnabled());
	ui.cb_displayDensity->setChecked(widget->isDensityDisplayEnabled());
	ui.tabs_displayoptions->setTabEnabled(0, widget->isEllipsoidDisplayEnabled() || widget->isGMPositionsDisplayEnabled());
	ui.tabs_displayoptions->setTabEnabled(1, widget->isDensityDisplayEnabled());
	ui.tabs_displayoptions->setTabEnabled(2, widget->isIsosurfaceDisplayEnabled());

	auto ellrenderer = widget->getGMIsoellipsoidRenderer();
	ui.spin_ellmin->setValue(ellrenderer->getEllMin());
	ui.spin_ellmin->setEnabled(ellrenderer->getRangeMode() == GMColorRangeMode::RANGE_MANUAL);
	ui.spin_ellmax->setValue(ellrenderer->getEllMax());
	ui.spin_ellmax->setEnabled(ellrenderer->getRangeMode() == GMColorRangeMode::RANGE_MANUAL);
	ui.co_ellrangemode->setCurrentIndex((int)ellrenderer->getRangeMode() - 1);
	ui.co_ellrendermode->setCurrentIndex((int)ellrenderer->getRenderMode() - 1);

	auto densrenderer = widget->getGMDensityRenderer();
	//ui.spin_dscalemin->setValue(densrenderer->getDensityMin() * 100);
	//ui.spin_dscalemax->setValue(densrenderer->getDensityMax() * 100);
	//ui.sl_dscalepercentage->setValue(100.0 - densrenderer->getDensityMax() / 1000.0);
	//ui.cb_dscaleauto->setChecked(densrenderer->getDensityAuto());
	//ui.cb_dscalecutoff->setChecked(densrenderer->getDensityCutoff());
	ui.co_densrendermode->setCurrentIndex((int)densrenderer->getRenderMode() - 1);
	ui.spin_accthreshold->setValue(densrenderer->getAccelerationThreshold() * 100);
	ui.spin_accthreshold->setEnabled(!densrenderer->getAccelerationThresholdAuto());
	ui.cb_accthauto->setChecked(densrenderer->getAccelerationThresholdAuto());
	ui.cb_dlog->setChecked(densrenderer->getLogarithmic());

	auto isosurfacerenderer = widget->getGMIsosurfaceRenderer();
	ui.spin_isovalue->setValue(isosurfacerenderer->getIsolevel());
	ui.spin_isoslidermax->setValue(0.001);
	ui.sl_isovalue->setValue(isosurfacerenderer->getIsolevel() / 0.001 * 100);


	(void)connect(ui.loadPointcloudAction, SIGNAL(triggered()), this, SLOT(slotLoadPointcloud()));
	(void)connect(ui.loadMixtureModelAction, SIGNAL(triggered()), this, SLOT(slotLoadMixtureModel()));
	(void)connect(ui.loadPureMixtureAction, SIGNAL(triggered()), this, SLOT(slotLoadPureMixture()));
	(void)connect(ui.loadLineAction, SIGNAL(triggered()), this, SLOT(slotLoadLine()));

	(void)connect(ui.chooseLineDirectoryAction, SIGNAL(triggered()), this, SLOT(slotChooseLineDirectory()));
	(void)connect(ui.openGLWidget, SIGNAL(gaussianSelected(int)), this, SLOT(slotGaussianSelected(int)));

	(void)connect(ui.cb_displayPointcloud, SIGNAL(clicked(bool)), this, SLOT(slotDisplayOptionsChanged()));
	(void)connect(ui.cb_displayGMPositions, SIGNAL(clicked(bool)), this, SLOT(slotDisplayOptionsChanged()));
	(void)connect(ui.cb_displayEllipsoids, SIGNAL(clicked(bool)), this, SLOT(slotDisplayOptionsChanged()));
	(void)connect(ui.cb_displayIsosurface, SIGNAL(clicked(bool)), this, SLOT(slotIsovalueDisplayOptionsChanged()));
	(void)connect(ui.cb_displayDensity, SIGNAL(clicked(bool)), this, SLOT(slotDisplayOptionsChanged()));

	(void)connect(ui.spin_ellmin, SIGNAL(valueChanged(double)), this, SLOT(slotEllValuesChanged()));
	(void)connect(ui.spin_ellmax, SIGNAL(valueChanged(double)), this, SLOT(slotEllValuesChanged()));
	(void)connect(ui.co_ellrangemode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotEllAutoValueChanged()));

	(void)connect(ui.spin_dscalemin, SIGNAL(valueChanged(double)), this, SLOT(slotDensityValuesChanged()));
	(void)connect(ui.spin_dscalemax, SIGNAL(valueChanged(double)), this, SLOT(slotDensityValuesChanged()));
	//(void)connect(ui.cb_dscalecutoff, SIGNAL(stateChanged(int)), this, SLOT(slotDensityValuesChanged()));
	//(void)connect(ui.cb_dscaleauto, SIGNAL(stateChanged(int)), this, SLOT(slotDensityAutoChanged()));
	(void)connect(ui.sl_dscalepercentage, SIGNAL(valueChanged(int)), this, SLOT(slotDensitySliderChanged()));
	(void)connect(ui.cb_dlog, SIGNAL(stateChanged(int)), this, SLOT(slotDensityLogModeChanged()));

	(void)connect(ui.co_densrendermode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDensityRenderModeChanged()));
	(void)connect(ui.co_ellrendermode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotEllipsoidRenderModeChanged()));
	(void)connect(ui.spin_accthreshold, SIGNAL(valueChanged(double)), this, SLOT(slotAccelerationThresholdChanged()));
	(void)connect(ui.cb_accthauto, SIGNAL(stateChanged(int)), this, SLOT(slotAccelerationThreshAutoChanged()));

	(void)connect(ui.spin_isovalue, SIGNAL(valueChanged(double)), this, SLOT(slotIsoValueChanged()));
	(void)connect(ui.spin_isoslidermax, SIGNAL(valueChanged(double)), this, SLOT(slotIsoSliderChanged()));
	(void)connect(ui.sl_isovalue, SIGNAL(valueChanged(int)), this, SLOT(slotIsoSliderChanged()));

	(void)connect(ui.openGLWidget, SIGNAL(frameSwapped()), this, SLOT(slotPostRender()));
}

void VisualizerWindow::slotLoadPointcloud() {
	QString filename = QFileDialog::getOpenFileName(this, "Load Pointcloud", openPcDirectory, "*.off");
	if (!filename.isNull()) {
		openPcDirectory = filename.left(std::max(filename.lastIndexOf("/"), filename.lastIndexOf("\\")));
		auto newPC = DataLoader::readPCDfromOFF(filename, false);
		if (newPC) {
			bool hasoldpc = pointcloud != nullptr;
			pointcloud = std::move(newPC);
			ui.openGLWidget->getPointCloudRenderer()->setPointCloud(pointcloud.get());
			if (ui.openGLWidget->isPointDisplayEnabled() && (!hasoldpc || !ui.openGLWidget->isGMVisibleInAnyWay() || !mixture)) {
				ui.openGLWidget->getCamera()->setPositionByBoundingBox(pointcloud->getBBMin(), pointcloud->getBBMax());
			}
		}
	}
}

void VisualizerWindow::slotLoadMixtureModel()
{
	bool hasprevmix = (mixture.get() != nullptr);
	QString filename = QFileDialog::getOpenFileName(this, "Load Mixture Model", openGmDirectory, "*.ply");
	if (!filename.isNull()) {
		int slashidx = std::max(filename.lastIndexOf("/"), filename.lastIndexOf("\\"));
		openGmDirectory = filename.left(slashidx);
		auto newGauss = DataLoader::readGMfromPLY<DECIMAL_TYPE>(filename, true, false);
		if (newGauss) {
			mixture = std::move(newGauss);
			ui.openGLWidget->setMixture(mixture.get());
			auto isoren = ui.openGLWidget->getGMIsoellipsoidRenderer();
			ui.spin_ellmin->blockSignals(true);
			ui.spin_ellmax->blockSignals(true);
			ui.spin_ellmin->setValue(isoren->getEllMin());
			ui.spin_ellmax->setValue(isoren->getEllMax());
			ui.spin_ellmin->blockSignals(false);
			ui.spin_ellmax->blockSignals(false);
			lineDirectory = openGmDirectory;
			QRegularExpression re("pcgmm-0-\\d{5}.ply$");
			if (re.match(filename).hasMatch()) {
				int id = filename.mid(filename.length() - 9, 5).toInt();
				ui.openGLWidget->getLineRenderer()->setMaxIteration(id);
			}
			else {
				ui.openGLWidget->getLineRenderer()->setMaxIteration(-1);
			}
			auto densrenderer = ui.openGLWidget->getGMDensityRenderer();
			if (!ui.openGLWidget->isPointDisplayEnabled() || pointcloud == nullptr) {
				QVector3D min, max;
				mixture->computePositionsBoundingBox(min, max);
				ui.openGLWidget->getCamera()->setPositionByBoundingBox(min, max);
			}
			slotDensitySliderChanged();
			setWindowTitle("GMVis: " + filename.right(filename.length() - slashidx - 1));
			ui.openGLWidget->update();
		}
	}
}

void VisualizerWindow::slotLoadPureMixture()
{
	bool hasprevmix = (mixture.get() != nullptr);
	QString filename = QFileDialog::getOpenFileName(this, "Load Mixture", openGmDirectory, "*.ply");
	if (!filename.isNull()) {
		int slashidx = std::max(filename.lastIndexOf("/"), filename.lastIndexOf("\\"));
		openGmDirectory = filename.left(slashidx);
		auto newGauss = DataLoader::readGMfromPLY<DECIMAL_TYPE>(filename, false, false);
		if (newGauss) {
			mixture = std::move(newGauss);
			ui.openGLWidget->setMixture(mixture.get());
			auto isoren = ui.openGLWidget->getGMIsoellipsoidRenderer();
			ui.spin_ellmin->blockSignals(true);
			ui.spin_ellmax->blockSignals(true);
			ui.spin_ellmin->setValue(isoren->getEllMin());
			ui.spin_ellmax->setValue(isoren->getEllMax());
			ui.spin_ellmin->blockSignals(false);
			ui.spin_ellmax->blockSignals(false);
			lineDirectory = openGmDirectory;
			QRegularExpression re("pcgmm-0-\\d{5}.ply$");
			if (re.match(filename).hasMatch()) {
				int id = filename.mid(filename.length() - 9, 5).toInt();
				ui.openGLWidget->getLineRenderer()->setMaxIteration(id);
			}
			else {
				ui.openGLWidget->getLineRenderer()->setMaxIteration(-1);
			}
			auto densrenderer = ui.openGLWidget->getGMDensityRenderer();
			if (!ui.openGLWidget->isPointDisplayEnabled() || pointcloud == nullptr) {
				QVector3D min, max;
				mixture->computePositionsBoundingBox(min, max);
				ui.openGLWidget->getCamera()->setPositionByBoundingBox(min, max);
			}
			slotDensitySliderChanged();	//update values
			setWindowTitle("GMVis: " + filename.right(filename.length() - slashidx - 1));
			ui.openGLWidget->update();
		}
	}
}

void gmvis::ui::VisualizerWindow::slotLoadLine()
{
	QString filename = QFileDialog::getOpenFileName(this, "Load Line", openGmDirectory, "*.txt;*.bin");
	if (!filename.isNull()) {
		std::unique_ptr<LineStrip> newLine;
		if (filename.endsWith(".txt")) {
			newLine = DataLoader::readLSfromTXT(filename);
		}
		else {
			newLine = DataLoader::readLSfromBIN(filename);
		}
		if (newLine && newLine->getDataSize() > 0) {
			line = std::move(newLine);
			ui.openGLWidget->getLineRenderer()->setLineStrip(line.get());
		}
		else {
			QMessageBox::critical(this, "Empty Line", "Read-in Line is empty or does not exist");
		}
	}
}

void gmvis::ui::VisualizerWindow::slotChooseLineDirectory()
{
	lineDirectory = QFileDialog::getExistingDirectory(this, "Open Line Directory");
}

void gmvis::ui::VisualizerWindow::slotGaussianSelected(int index)
{
	if (lineDirectory.isNull()) return;

	if (index == -1) {
		line = nullptr;
		ui.openGLWidget->getLineRenderer()->setLineStrip(nullptr);
		ui.openGLWidget->repaint();
	}

	QString path = lineDirectory + "/pos-g" + std::to_string(index).c_str() + ".bin";
	auto newLine = DataLoader::readLSfromBIN(path);
	if (!newLine) {
		QString path = lineDirectory + "/pos-g" + std::to_string(index).c_str() + ".txt";
		newLine = DataLoader::readLSfromTXT(path);
	}
	if (newLine && newLine->getDataSize() > 0) {
		line = std::move(newLine);
		ui.openGLWidget->getLineRenderer()->setLineStrip(line.get());
		ui.openGLWidget->update();
	}
	else {
		QMessageBox::critical(this, "Empty Line", "Read-in Line is empty or does not exist");
	}
}

void VisualizerWindow::slotDisplayOptionsChanged()
{
	ui.openGLWidget->setPointDisplayEnabled(ui.cb_displayPointcloud->isChecked());
	ui.openGLWidget->setGMPositionsDisplayEnabled(ui.cb_displayGMPositions->isChecked());
	ui.openGLWidget->setEllipsoidDisplayEnabled(ui.cb_displayEllipsoids->isChecked());
	ui.openGLWidget->setDensityDisplayEnabled(ui.cb_displayDensity->isChecked());

	ui.tabs_displayoptions->setTabEnabled(0, ui.cb_displayGMPositions->isChecked() || ui.cb_displayEllipsoids->isChecked());
	ui.tabs_displayoptions->setTabEnabled(1, ui.cb_displayDensity->isChecked());
	ui.tabs_displayoptions->setTabEnabled(2, false);

	ui.openGLWidget->update();
}

void gmvis::ui::VisualizerWindow::slotIsovalueDisplayOptionsChanged()
{
	ui.openGLWidget->setIsosurfaceDisplayEnabled(ui.cb_displayIsosurface->isChecked());
	ui.cb_displayPointcloud->blockSignals(true);
	ui.cb_displayGMPositions->blockSignals(true);
	ui.cb_displayEllipsoids->blockSignals(true);
	ui.cb_displayDensity->blockSignals(true);
	if (ui.cb_displayIsosurface->isChecked()) {
		ui.cb_displayPointcloud->setChecked(false);
		ui.cb_displayGMPositions->setChecked(false);
		ui.cb_displayEllipsoids->setChecked(false);
		ui.cb_displayDensity->setChecked(false);
		ui.tabs_displayoptions->setTabEnabled(1, false);
		ui.tabs_displayoptions->setTabEnabled(0, false);
	}
	else {
		ui.cb_displayPointcloud->setChecked(ui.openGLWidget->isPointDisplayEnabled());
		ui.cb_displayGMPositions->setChecked(ui.openGLWidget->isGMPositionsDisplayEnabled());
		ui.cb_displayEllipsoids->setChecked(ui.openGLWidget->isEllipsoidDisplayEnabled());
		ui.cb_displayDensity->setChecked(ui.openGLWidget->isDensityDisplayEnabled());
		ui.tabs_displayoptions->setTabEnabled(1, ui.cb_displayDensity->isChecked());
		ui.tabs_displayoptions->setTabEnabled(0, ui.cb_displayGMPositions->isChecked() || ui.cb_displayEllipsoids->isChecked());
	}
	ui.cb_displayPointcloud->blockSignals(false);
	ui.cb_displayGMPositions->blockSignals(false);
	ui.cb_displayEllipsoids->blockSignals(false);
	ui.cb_displayDensity->blockSignals(false);

	ui.cb_displayPointcloud->setDisabled(ui.cb_displayIsosurface->isChecked());
	ui.cb_displayGMPositions->setDisabled(ui.cb_displayIsosurface->isChecked());
	ui.cb_displayEllipsoids->setDisabled(ui.cb_displayIsosurface->isChecked());
	ui.cb_displayDensity->setDisabled(ui.cb_displayIsosurface->isChecked());
	ui.tabs_displayoptions->setTabEnabled(2, ui.cb_displayIsosurface->isChecked());

	ui.openGLWidget->update();
}

void VisualizerWindow::slotEllValuesChanged()
{
	auto renderer1 = ui.openGLWidget->getGMIsoellipsoidRenderer();
	renderer1->setEllMin(ui.spin_ellmin->value());
	renderer1->setEllMax(ui.spin_ellmax->value());
	renderer1->updateColors();
	auto renderer2 = ui.openGLWidget->getGMPositionsRenderer();
	renderer2->setEllMin(ui.spin_ellmin->value());
	renderer2->setEllMax(ui.spin_ellmax->value());
	renderer2->updateColors();
	ui.openGLWidget->update();
}

void VisualizerWindow::slotEllAutoValueChanged()
{
	auto renderer1 = ui.openGLWidget->getGMIsoellipsoidRenderer();
	auto renderer2 = ui.openGLWidget->getGMPositionsRenderer();
	GMColorRangeMode val = GMColorRangeMode(ui.co_ellrangemode->currentIndex() + 1);
	renderer1->setRangeMode(val);
	renderer1->updateColors();
	renderer2->setRangeMode(val);
	renderer2->updateColors();
	if (val != GMColorRangeMode::RANGE_MANUAL) {
		ui.spin_ellmin->blockSignals(true);
		ui.spin_ellmax->blockSignals(true);
		ui.spin_ellmin->setValue(renderer1->getEllMin());
		ui.spin_ellmax->setValue(renderer1->getEllMax());
		ui.spin_ellmin->setEnabled(false);
		ui.spin_ellmax->setEnabled(false);
		ui.spin_ellmin->blockSignals(false);
		ui.spin_ellmax->blockSignals(false);
		renderer1->updateColors();
		renderer2->updateColors();
	}
	else {
		ui.spin_ellmin->setEnabled(true);
		ui.spin_ellmax->setEnabled(true);
	}
	ui.openGLWidget->update();
}

void VisualizerWindow::slotEllipsoidRenderModeChanged()
{
	auto renderer1 = ui.openGLWidget->getGMIsoellipsoidRenderer();
	auto renderer2 = ui.openGLWidget->getGMPositionsRenderer();
	renderer1->setRenderMode(GMColoringRenderMode(ui.co_ellrendermode->currentIndex() + 1));
	renderer1->updateColors();
	renderer2->setRenderMode(GMColoringRenderMode(ui.co_ellrendermode->currentIndex() + 1));
	renderer2->updateColors();
	ui.spin_ellmin->setValue(renderer1->getEllMin());
	ui.spin_ellmax->setValue(renderer1->getEllMax());
	ui.openGLWidget->update();
}

void gmvis::ui::VisualizerWindow::slotDensitySliderChanged()
{
	bool dauto = false;// ui.cb_dscaleauto->isChecked();
	if (dauto)
	{
		slotDensityAutoChanged();
	}
	else
	{
		//this will trigger slotDensityValuesChanged for update of values in renderer
		auto renderer = ui.openGLWidget->getGMDensityRenderer();
		float newval = renderer->getSuggestedDensityMaxLimit() * 0.01 * (100 - ui.sl_dscalepercentage->value());
		if (newval > ui.spin_dscalemin->value())
		{
			ui.spin_dscalemax->setValue(newval);
		}
	}
}

void VisualizerWindow::slotDensityValuesChanged()
{
	auto renderer = ui.openGLWidget->getGMDensityRenderer();
	renderer->setDensityMin(ui.spin_dscalemin->value() * 0.01);
	renderer->setDensityMax(ui.spin_dscalemax->value() * 0.01);
	//renderer->setDensityCutoff(ui.cb_dscalecutoff->isChecked());
	//set Slider Value
	if (ui.cb_accthauto->isChecked()) {
		ui.spin_accthreshold->blockSignals(true);
		ui.spin_accthreshold->setValue(renderer->getAccelerationThreshold() * 100);
		ui.spin_accthreshold->blockSignals(false);
		slotAccelerationThresholdChanged(false);
	}
	//if (!ui.cb_dscaleauto->isChecked()) {
	ui.sl_dscalepercentage->blockSignals(true);
	ui.sl_dscalepercentage->setValue(100 - 100 * (ui.spin_dscalemax->value() / renderer->getSuggestedDensityMaxLimit()));
	ui.sl_dscalepercentage->blockSignals(false);
	ui.openGLWidget->update();
	//}
}

void VisualizerWindow::slotDensityAutoChanged()
{
	auto renderer = ui.openGLWidget->getGMDensityRenderer();
	bool dauto = false;// ui.cb_dscaleauto->isChecked();
	renderer->setDensityAuto(dauto);
	if (dauto)
	{
		renderer->setDensityAutoPercentage(1 - ui.sl_dscalepercentage->value() * 0.005);
	}
	ui.spin_dscalemax->setEnabled(!dauto);
	ui.spin_dscalemin->setEnabled(!dauto);
	ui.openGLWidget->update();
}

void gmvis::ui::VisualizerWindow::slotDensityLogModeChanged()
{
	auto renderer = ui.openGLWidget->getGMDensityRenderer();
	auto log = ui.cb_dlog->isChecked();
	renderer->setLogarithmic(log);
	ui.spin_dscalemin->blockSignals(true);
	ui.spin_dscalemax->blockSignals(true);
	//if auto off
	//ui.sl_dscalepercentage->setValue()
	ui.spin_dscalemin->setValue(renderer->getDensityMin() * 100);
	ui.spin_dscalemax->setValue(renderer->getDensityMax() * 100);
	ui.spin_isoslidermax->setEnabled(!log);
	ui.spin_dscalemin->blockSignals(false);
	ui.spin_dscalemax->blockSignals(false);
	ui.openGLWidget->update();
}

void VisualizerWindow::slotDensityRenderModeChanged()
{
	auto renderer = ui.openGLWidget->getGMDensityRenderer();
	renderer->setRenderMode(GMDensityRenderMode(ui.co_densrendermode->currentIndex() + 1));
	ui.openGLWidget->update();
}

void VisualizerWindow::slotAccelerationThresholdChanged(bool update)
{
	auto renderer = ui.openGLWidget->getGMDensityRenderer();
	renderer->setAccelerationThreshold(0.01 * ui.spin_accthreshold->value());
	renderer->updateAccelerationData();
	//if (!ui.cb_accthauto->isChecked() || !ui.cb_dscaleauto->isChecked()) {
	if (update) {
		ui.openGLWidget->update();
	}
}

void VisualizerWindow::slotAccelerationThreshAutoChanged()
{
	auto renderer = ui.openGLWidget->getGMDensityRenderer();
	bool autoth = ui.cb_accthauto->isChecked();
	renderer->setAccelerationThresholdAuto(autoth);
	ui.spin_accthreshold->setEnabled(!autoth);
	//This will call slotAccelerationThresholdChanged(true) and automatically update the window
	ui.spin_accthreshold->setValue(renderer->getAccelerationThreshold() * 100);
}

void gmvis::ui::VisualizerWindow::slotIsoValueChanged()
{
	auto renderer = ui.openGLWidget->getGMIsosurfaceRenderer();
	renderer->setIsolevel(ui.spin_isovalue->value());
	ui.sl_isovalue->blockSignals(true);
	ui.sl_isovalue->setValue(round(ui.spin_isovalue->value() / ui.spin_isoslidermax->value() * 100));
	ui.sl_isovalue->blockSignals(false);
	ui.openGLWidget->update();
}

void gmvis::ui::VisualizerWindow::slotIsoSliderChanged()
{
	//this triggers slotIsoValueChanged, so widget will be updated
	ui.spin_isovalue->setValue(ui.sl_isovalue->value() / 100.0 * ui.spin_isoslidermax->value());
}

void VisualizerWindow::slotPostRender()
{
	/*if (ui.cb_dscaleauto->isChecked()) {
		auto renderer = ui.openGLWidget->getGMDensityRenderer();
		ui.spin_dscalemin->blockSignals(true);
		ui.spin_dscalemax->blockSignals(true);
		ui.spin_dscalemin->setValue(renderer->getDensityMin() * 100);
		ui.spin_dscalemax->setValue(renderer->getDensityMax() * 100);
		ui.spin_dscalemin->blockSignals(false);
		ui.spin_dscalemax->blockSignals(false);
	}*/
}
