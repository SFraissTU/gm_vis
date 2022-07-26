#include "gmvis/ui/VisualizerWindow.h"
#include <qfiledialog.h>
#include <QDoubleValidator>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QMessageBox>
#include "GaussianListItem.h"

using namespace gmvis::ui;
using namespace gmvis::core;

VisualizerWindow::VisualizerWindow(QWidget *parent)
	: QMainWindow(parent), config("SFraissTU", "GMVis")
{
	ui.setupUi(this);
	
    // settings
    m_scaling_actual2ui = config.value("scaling_actual2ui_is_100", true).toBool() ? 100 : 1;
    ui.cb_actual_x100_to_ui_scaling->setChecked(config.value("scaling_actual2ui_is_100", true).toBool());

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
    //ui.spin_dscalemin->setValue(densrenderer->getDensityMin() * m_scaling_actual2ui);
    //ui.spin_dscalemax->setValue(densrenderer->getDensityMax() * m_scaling_actual2ui);
    //ui.sl_dscalepercentage->setValue(100.0 - densrenderer->getDensityMax() / 1000.0);
	//ui.cb_dscaleauto->setChecked(densrenderer->getDensityAuto());
	//ui.cb_dscalecutoff->setChecked(densrenderer->getDensityCutoff());
	ui.co_densrendermode->setCurrentIndex((int)densrenderer->getRenderMode() - 1);
    ui.spin_accthreshold->setValue(densrenderer->getAccelerationThreshold() * m_scaling_actual2ui);
	ui.spin_accthreshold->setEnabled(!densrenderer->getAccelerationThresholdAuto());
	ui.cb_accthauto->setChecked(densrenderer->getAccelerationThresholdAuto());
	ui.cb_dlog->setChecked(densrenderer->getLogarithmic());

	auto isosurfacerenderer = widget->getGMIsosurfaceRenderer();
	ui.spin_isovalue->setValue(isosurfacerenderer->getIsolevel());
	ui.spin_isoslidermax->setValue(0.001);
    ui.sl_isovalue->setValue(isosurfacerenderer->getIsolevel() / 0.001 * 100);

	auto dbgmenu = ui.menuBar->addMenu("Options");
	auto fpsbtn = dbgmenu->addAction("Print Render Times");
	fpsbtn->setCheckable(true);
	(void)connect(fpsbtn, SIGNAL(triggered()), this, SLOT(slotToggleFPS()));


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

	(void)connect(ui.btn_camReset, SIGNAL(clicked(bool)), this, SLOT(slotResetCamera()));
	(void)connect(ui.openGLWidget, SIGNAL(cameraMoved(core::Camera*)), this, SLOT(slotCameraMoved(core::Camera*)));

	(void)connect(ui.spin_ellmin, SIGNAL(valueChanged(double)), this, SLOT(slotEllValuesChanged()));
	(void)connect(ui.spin_ellmax, SIGNAL(valueChanged(double)), this, SLOT(slotEllValuesChanged()));
	(void)connect(ui.co_ellrangemode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotEllAutoValueChanged()));

	(void)connect(ui.spin_dscalemin, SIGNAL(valueChanged(double)), this, SLOT(slotDensityValuesChanged()));
	(void)connect(ui.spin_dscalemax, SIGNAL(valueChanged(double)), this, SLOT(slotDensityValuesChanged()));
	//(void)connect(ui.cb_dscalecutoff, SIGNAL(stateChanged(int)), this, SLOT(slotDensityValuesChanged()));
	//(void)connect(ui.cb_dscaleauto, SIGNAL(stateChanged(int)), this, SLOT(slotDensityAutoChanged()));
	(void)connect(ui.sl_dscalepercentage, SIGNAL(valueChanged(int)), this, SLOT(slotDensitySliderChanged()));
	(void)connect(ui.cb_dlog, SIGNAL(stateChanged(int)), this, SLOT(slotDensityLogModeChanged()));
	(void)connect(ui.btn_densityreset, SIGNAL(clicked()), this, SLOT(slotResetDensity()));

	(void)connect(ui.co_densrendermode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDensityRenderModeChanged()));
	(void)connect(ui.co_ellrendermode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotEllipsoidRenderModeChanged()));
	(void)connect(ui.spin_accthreshold, SIGNAL(valueChanged(double)), this, SLOT(slotAccelerationThresholdChanged()));
	(void)connect(ui.cb_accthauto, SIGNAL(stateChanged(int)), this, SLOT(slotAccelerationThreshAutoChanged()));

	(void)connect(ui.spin_isovalue, SIGNAL(valueChanged(double)), this, SLOT(slotIsoValueChanged()));
	(void)connect(ui.spin_isoslidermax, SIGNAL(valueChanged(double)), this, SLOT(slotIsoSliderChanged()));
	(void)connect(ui.sl_isovalue, SIGNAL(valueChanged(int)), this, SLOT(slotIsoSliderChanged()));

	(void)connect(ui.list_gaussians, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(slotListGaussianSelected(QListWidgetItem*)));
	(void)connect(ui.btn_pick, SIGNAL(clicked(bool)), this, SLOT(slotTogglePickGaussian(bool)));
	(void)connect(ui.btn_clearsel, SIGNAL(clicked(bool)), this, SLOT(slotClearSelection()));

	(void)connect(ui.btn_hidezero, SIGNAL(clicked(bool)), this, SLOT(slotHideZeroGaussians(bool)));
	(void)connect(ui.btn_hideinvalid, SIGNAL(clicked(bool)), this, SLOT(slotHideInvalidGaussians(bool)));
	(void)connect(ui.btn_backgr, SIGNAL(clicked()), this, SLOT(slotToggleBackground()));

	(void)connect(ui.openGLWidget, SIGNAL(frameSwapped()), this, SLOT(slotPostRender()));

    connect(ui.btn_density_lock_symmetry, &QPushButton::toggled, this, &VisualizerWindow::slotDensityValuesChanged);
    connect(ui.cb_isoEllipsoidRender, &QCheckBox::stateChanged, this, &VisualizerWindow::slotEllipsoidCovOrIsoChanged);
    connect(ui.spin_isoEllipsoidThreshold, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &VisualizerWindow::slotEllipsoidCovOrIsoChanged);
}

void VisualizerWindow::slotLoadPointcloud() {
	QString filename = QFileDialog::getOpenFileName(this, "Load Pointcloud", config.value("openPcDirectory").toString(), "OFF (*.off);;All Files(*.*)");
	if (!filename.isNull()) {
		config.setValue("openPcDirectory", filename.left(std::max(filename.lastIndexOf("/"), filename.lastIndexOf("\\"))));
		ui.le_pcfile->setText(filename);
		QString error;
		auto newPC = DataLoader::readPCDfromOFF(filename, false, error);
		if (!error.isEmpty()) {
			if (newPC) {
				QMessageBox::warning(this, "Warning", error);
			}
			else {
				QMessageBox::critical(this, "Error", error);
			}
		}
		if (newPC) {
			bool hasoldpc = pointcloud != nullptr;
			pointcloud = std::move(newPC);
			ui.openGLWidget->getPointCloudRenderer()->setPointCloud(pointcloud.get());
			if (ui.openGLWidget->isPointDisplayEnabled() && (!hasoldpc || !ui.openGLWidget->isGMVisibleInAnyWay() || !mixture)) {
				auto bbmin = pointcloud->getBBMin();
				auto bbmax = pointcloud->getBBMax();
				ui.openGLWidget->getCamera()->setPositionByBoundingBox(bbmin, bbmax);
				ui.openGLWidget->getCamera()->setTranslationSpeedByBoundingBox(bbmin, bbmax);
				slotCameraMoved(ui.openGLWidget->getCamera());
			}
			ui.openGLWidget->update();
		}
	}
}

void VisualizerWindow::slotLoadMixtureModel()
{
	bool hasprevmix = (mixture.get() != nullptr);
	QString filename = QFileDialog::getOpenFileName(this, "Load Mixture Model", config.value("openGmDirectory").toString(), "PLY (*.ply);;All Files(*.*)");
	if (!filename.isNull()) {
		QString error;
		auto newGauss = DataLoader::readGMfromPLY<DECIMAL_TYPE>(filename, true, false, error);
		if (!error.isEmpty()) {
			if (newGauss) {
				QMessageBox::warning(this, "Warning", error);
			}
			else {
				QMessageBox::critical(this, "Error", error);
			}
		}
		setNewMixture(newGauss, filename);
	}
}

void VisualizerWindow::slotLoadPureMixture()
{
	bool hasprevmix = (mixture.get() != nullptr);
	QString filename = QFileDialog::getOpenFileName(this, "Load Mixture", config.value("openGmDirectory").toString(), "*PLY (*.ply);;All Files(*.*)");
	if (!filename.isNull()) {
        loadPureMixture(filename);
	}
}

void gmvis::ui::VisualizerWindow::slotLoadLine()
{
	QString filename = QFileDialog::getOpenFileName(this, "Load Line", config.value("openGmDirectory").toString(), "*.txt;*.bin");
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
	ui.list_gaussians->setCurrentRow(mixture->gaussIndexFromEnabledGaussIndex(index));
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

void VisualizerWindow::slotEllipsoidCovOrIsoChanged()
{
    ui.openGLWidget->getGMIsoellipsoidRenderer()->setDrawIsoEllipsoids(ui.cb_isoEllipsoidRender->isChecked());
    ui.openGLWidget->getGMIsoellipsoidRenderer()->setIsoEllipsoidThreshold(ui.spin_isoEllipsoidThreshold->value());
    ui.openGLWidget->getGMIsoellipsoidRenderer()->updateMixture();
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
		auto renderer = ui.openGLWidget->getGMDensityRenderer();
		if (ui.cb_dlog->isChecked()) {
			double dlmin = renderer->getSuggestedDensityLogMinLimit();
			double dlmax = renderer->getSuggestedDensityLogMaxLimit();
            double newval = m_scaling_actual2ui * (dlmin + (dlmax - dlmin) * (ui.sl_dscalepercentage->value() / 100.0));
			if (newval < ui.spin_dscalemax->value()) {
				ui.spin_dscalemin->setValue(newval);
			}
		}
		else {
			//this will trigger slotDensityValuesChanged for update of values in renderer
            double newval = m_scaling_actual2ui * renderer->getSuggestedDensityMaxLimit() * (1.0 - ui.sl_dscalepercentage->value() / 100.);
			if (newval > ui.spin_dscalemin->value()) {
				ui.spin_dscalemax->setValue(newval);
			}
		}
	}
}

void VisualizerWindow::slotDensityValuesChanged()
{
    auto val_max = ui.spin_dscalemax->value() / m_scaling_actual2ui;
    auto val_min = ui.spin_dscalemin->value() / m_scaling_actual2ui;

	auto renderer = ui.openGLWidget->getGMDensityRenderer();
    renderer->setDensityMin(val_min);
    renderer->setDensityMax(val_max);
	//renderer->setDensityCutoff(ui.cb_dscalecutoff->isChecked());
	//set Slider Value
	if (ui.cb_accthauto->isChecked()) {
		ui.spin_accthreshold->blockSignals(true);
        ui.spin_accthreshold->setValue(renderer->getAccelerationThreshold() * m_scaling_actual2ui);
		ui.spin_accthreshold->blockSignals(false);
		slotAccelerationThresholdChanged(false);
	}
	//if (!ui.cb_dscaleauto->isChecked()) {
	if (ui.cb_dlog->isChecked()) {
		ui.sl_dscalepercentage->blockSignals(true);
		double dlmin = renderer->getSuggestedDensityLogMinLimit();
		double dlmax = renderer->getSuggestedDensityLogMaxLimit();
        double newpercval = (val_min - dlmin) / (dlmax - dlmin);
        ui.sl_dscalepercentage->setValue(int(100. * newpercval));   // always 0-100%
		ui.sl_dscalepercentage->blockSignals(false);
	}
	else {
		ui.sl_dscalepercentage->blockSignals(true);
        ui.sl_dscalepercentage->setValue(100 - int(100. * val_max / renderer->getSuggestedDensityMaxLimit()));
		ui.sl_dscalepercentage->blockSignals(false);
	}
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
    ui.spin_dscalemin->setValue(renderer->getDensityMin() * m_scaling_actual2ui);
    ui.spin_dscalemax->setValue(renderer->getDensityMax() * m_scaling_actual2ui);
	ui.spin_isoslidermax->setEnabled(!log);
	ui.spin_dscalemin->blockSignals(false);
	ui.spin_dscalemax->blockSignals(false);
	slotDensitySliderChanged();
	slotDensityValuesChanged();
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
	if (!ui.cb_accthauto->isChecked())
	{
        renderer->setAccelerationThreshold(ui.spin_accthreshold->value() / m_scaling_actual2ui);
	}
	renderer->updateAccelerationData();
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
    ui.spin_accthreshold->setValue(renderer->getAccelerationThreshold() * m_scaling_actual2ui);
}

void gmvis::ui::VisualizerWindow::slotIsoValueChanged()
{
	auto renderer = ui.openGLWidget->getGMIsosurfaceRenderer();
	renderer->setIsolevel(ui.spin_isovalue->value());
	ui.sl_isovalue->blockSignals(true);
    ui.sl_isovalue->setValue(round(ui.spin_isovalue->value() / ui.spin_isoslidermax->value() * m_scaling_actual2ui));
	ui.sl_isovalue->blockSignals(false);
	ui.openGLWidget->update();
}

void gmvis::ui::VisualizerWindow::slotIsoSliderChanged()
{
	//this triggers slotIsoValueChanged, so widget will be updated
    ui.spin_isovalue->setValue(ui.sl_isovalue->value() / m_scaling_actual2ui * ui.spin_isoslidermax->value());
}

void VisualizerWindow::slotPostRender()
{
	/*if (ui.cb_dscaleauto->isChecked()) {
		auto renderer = ui.openGLWidget->getGMDensityRenderer();
		ui.spin_dscalemin->blockSignals(true);
		ui.spin_dscalemax->blockSignals(true);
        ui.spin_dscalemin->setValue(renderer->getDensityMin() * m_scaling_actual2ui);
        ui.spin_dscalemax->setValue(renderer->getDensityMax() * m_scaling_actual2ui);
		ui.spin_dscalemin->blockSignals(false);
		ui.spin_dscalemax->blockSignals(false);
	}*/
}

void gmvis::ui::VisualizerWindow::slotResetDensity()
{
	auto densrenderer = ui.openGLWidget->getGMDensityRenderer();
	densrenderer->setMixture(mixture.get(), true);
	densrenderer->setLogarithmic(true);
	double dlmin = densrenderer->getSuggestedDensityLogMinLimit();
	double dlmax = densrenderer->getSuggestedDensityLogMaxLimit();
	densrenderer->setDensityMin(dlmin + (dlmax - dlmin) * 0.75);
	densrenderer->setDensityMax(dlmax);
	densrenderer->setLogarithmic(false);
	double dnmax = densrenderer->getSuggestedDensityMaxLimit();
	if (mixture->containsNegativeGaussians()) {
		densrenderer->setDensityMin(-dnmax * 0.25);
	}
	else {
		densrenderer->setDensityMin(0);
	}
	densrenderer->setDensityMax(dnmax * 0.25);
	densrenderer->setLogarithmic(ui.cb_dlog->isChecked());
	ui.sl_dscalepercentage->blockSignals(true);
	ui.spin_dscalemin->blockSignals(true);
	ui.spin_dscalemax->blockSignals(true);
	ui.sl_dscalepercentage->setValue(75);
    ui.spin_dscalemin->setValue(densrenderer->getDensityMin() * m_scaling_actual2ui);
    ui.spin_dscalemax->setValue(densrenderer->getDensityMax() * m_scaling_actual2ui);
	ui.sl_dscalepercentage->blockSignals(false);
	ui.spin_dscalemin->blockSignals(false);
	ui.spin_dscalemax->blockSignals(false);
	if (ui.cb_accthauto->isChecked()) {
		ui.spin_accthreshold->blockSignals(true);
        ui.spin_accthreshold->setValue(densrenderer->getAccelerationThreshold() * m_scaling_actual2ui);
		ui.spin_accthreshold->blockSignals(false);
		slotAccelerationThresholdChanged(false);
	}
	densrenderer->updateAccelerationData();
	ui.openGLWidget->update();
}

void gmvis::ui::VisualizerWindow::slotListGaussianSelected(QListWidgetItem* item)
{
	auto gitem = dynamic_cast<GaussianListItem*>(item);
	if (gitem) {
		ui.txt_output->setPlainText(gitem->getDescription());
		int selectedGaussian = mixture->enabledGaussIndexFromGaussIndex(gitem->getIndex());
		ui.openGLWidget->getGMIsoellipsoidRenderer()->setMarkedGaussian(selectedGaussian);
		ui.openGLWidget->getGMPositionsRenderer()->setMarkedGaussian(selectedGaussian);
	}
	else {
		ui.openGLWidget->getGMIsoellipsoidRenderer()->setMarkedGaussian(-1);
		ui.openGLWidget->getGMPositionsRenderer()->setMarkedGaussian(-1);
		ui.txt_output->setPlainText(QString());
	}
	if (!lineDirectory.isNull()) {
		if (gitem) {
			int index = gitem->getIndex();
			QString path = lineDirectory + "/pos-g" + std::to_string(index).c_str() + ".bin";
			auto newLine = DataLoader::readLSfromBIN(path);
			if (!newLine) {
				QString path = lineDirectory + "/pos-g" + std::to_string(index).c_str() + ".txt";
				newLine = DataLoader::readLSfromTXT(path);
			}
			if (newLine && newLine->getDataSize() > 0) {
				line = std::move(newLine);
				ui.openGLWidget->getLineRenderer()->setLineStrip(line.get());
			}
			else {
				line = nullptr;
				ui.openGLWidget->getLineRenderer()->setLineStrip(nullptr);
			}
		}
		else {
			line = nullptr;
			ui.openGLWidget->getLineRenderer()->setLineStrip(nullptr);
		}
	}
	ui.openGLWidget->update();
}

void gmvis::ui::VisualizerWindow::slotTogglePickGaussian(bool checked)
{
	ui.openGLWidget->setPickingEnabled(checked);
}

void gmvis::ui::VisualizerWindow::slotClearSelection()
{
	ui.list_gaussians->setCurrentRow(-1);
}

void gmvis::ui::VisualizerWindow::slotResetCamera()
{
	if (pointcloud)
	{
		auto bbmin = pointcloud->getBBMin();
		auto bbmax = pointcloud->getBBMax();
		ui.openGLWidget->getCamera()->setPositionByBoundingBox(bbmin, bbmax);
		ui.openGLWidget->getCamera()->setTranslationSpeedByBoundingBox(bbmin, bbmax);
		ui.openGLWidget->update();
	}
	else if (mixture)
	{
		QVector3D min, max;
		mixture->computeEllipsoidsBoundingBox(min, max);
		ui.openGLWidget->getCamera()->setPositionByBoundingBox(min, max);
		ui.openGLWidget->getCamera()->setTranslationSpeedByBoundingBox(min, max);
		ui.openGLWidget->update();
	}
	slotCameraMoved(ui.openGLWidget->getCamera());
}

void gmvis::ui::VisualizerWindow::slotCameraMoved(core::Camera* camera)
{
	std::stringstream stream;
	auto pos = camera->getPosition();
	auto lookat = camera->getLookAt();
	stream << "Camera Position: \n" << "  " << pos.x() << " / " << pos.y() << " / " << pos.z() << "\n";
	stream << "Looking At: \n" << "  " << lookat.x() << " / " << lookat.y() << " / " << lookat.z();
	ui.txt_caminfo->setPlainText(QString::fromStdString(stream.str()));
}

void gmvis::ui::VisualizerWindow::slotHideZeroGaussians(bool checked)
{
	mixture->setDisableZeroGaussians(checked);
	ui.openGLWidget->updateMixture();
	if (ui.list_gaussians->currentRow() != -1) {
		ui.list_gaussians->setCurrentRow(-1);
	}
	else {
		ui.openGLWidget->update();
	}
}

void gmvis::ui::VisualizerWindow::slotHideInvalidGaussians(bool checked)
{
	mixture->setDisableInvalidGaussians(checked);
	ui.openGLWidget->updateMixture();
	if (ui.list_gaussians->currentRow() != -1) {
		ui.list_gaussians->setCurrentRow(-1);
	}
	else {
		ui.openGLWidget->update();
	}
}

void gmvis::ui::VisualizerWindow::slotToggleBackground()
{
	whiteMode = !whiteMode;
	ui.openGLWidget->setWhiteMode(whiteMode);
	ui.openGLWidget->update();
}

void gmvis::ui::VisualizerWindow::slotToggleFPS()
{
    ui.openGLWidget->toggleFps();
}

void VisualizerWindow::loadPureMixture(const QString& filename)
{
    QString error;
    auto newGauss = DataLoader::readGMfromPLY<DECIMAL_TYPE>(filename, false, false, error);
    if (!error.isEmpty()) {
        if (newGauss) {
            QMessageBox::warning(this, "Warning", error);
        }
        else {
            QMessageBox::critical(this, "Error", error);
        }
    }
    setNewMixture(newGauss, filename);
}

void gmvis::ui::VisualizerWindow::setNewMixture(std::unique_ptr<core::GaussianMixture<DECIMAL_TYPE>>& newGauss, const QString& fileLoadedFrom)
{
	int slashidx = std::max(fileLoadedFrom.lastIndexOf("/"), fileLoadedFrom.lastIndexOf("\\"));
	config.setValue("openGmDirectory", fileLoadedFrom.left(slashidx));
	bool prevmix = mixture.get() != nullptr;
	if (newGauss)
	{
		ui.le_mixfile->setText(fileLoadedFrom);
		mixture = std::move(newGauss);
		ui.openGLWidget->setMixture(mixture.get(), !ui.btn_lockdspoptions->isChecked() || !prevmix);
		auto isoren = ui.openGLWidget->getGMIsoellipsoidRenderer();
		ui.spin_ellmin->blockSignals(true);
		ui.spin_ellmax->blockSignals(true);
		ui.spin_ellmin->setValue(isoren->getEllMin());
		ui.spin_ellmax->setValue(isoren->getEllMax());
		ui.spin_ellmin->blockSignals(false);
		ui.spin_ellmax->blockSignals(false);
		lineDirectory = fileLoadedFrom.left(slashidx);
		QRegularExpression re("pcgmm-0-\\d{5}.ply$");
		if (re.match(fileLoadedFrom).hasMatch()) {
			int id = fileLoadedFrom.mid(fileLoadedFrom.length() - 9, 5).toInt();
			ui.openGLWidget->getLineRenderer()->setMaxIteration(id);
		}
		else {
			ui.openGLWidget->getLineRenderer()->setMaxIteration(-1);
		}
		auto densrenderer = ui.openGLWidget->getGMDensityRenderer();
		if (!ui.openGLWidget->isPointDisplayEnabled() || pointcloud == nullptr) {
			QVector3D min, max;
			mixture->computeEllipsoidsBoundingBox(min, max);
			ui.openGLWidget->getCamera()->setPositionByBoundingBox(min, max);
			ui.openGLWidget->getCamera()->setTranslationSpeedByBoundingBox(min, max);
			slotCameraMoved(ui.openGLWidget->getCamera());
		}
		//Fill Gaussian List
		ui.list_gaussians->clear();
		for (int i = 0; i < mixture->numberOfGaussians(); ++i)
		{
			GaussianListItem* item = new GaussianListItem(i, (*mixture)[i]);
			ui.list_gaussians->addItem(item);
		}
		setWindowTitle("GMVis: " + fileLoadedFrom.right(fileLoadedFrom.length() - slashidx - 1));
		if (!ui.btn_lockdspoptions->isChecked() || !prevmix) {
			slotResetDensity();	//calls update()
		}
		else {
			ui.openGLWidget->update();
		}
	}
}

void gmvis::ui::VisualizerWindow::on_btn_adams_scale_clicked()
{
    ui.openGLWidget->makeCurrent();
    auto min_max_density = ui.openGLWidget->getGMDensityRenderer()->find_min_max_density();
    if (ui.btn_density_lock_symmetry->isChecked() && !ui.cb_dlog->isChecked()) {
        auto val_min = min_max_density.first;
        auto val_max = min_max_density.second;
        val_max = std::max(std::abs(val_min), val_max);
        val_min = - val_max;
        min_max_density.first = val_min;
        min_max_density.second = val_max;

        ui.spin_dscalemax->setValue(double(min_max_density.second) * m_scaling_actual2ui);
    }
    else {
        ui.spin_dscalemin->blockSignals(true);
        ui.spin_dscalemin->setValue(double(min_max_density.first) * m_scaling_actual2ui);
        ui.spin_dscalemin->blockSignals(false);
        ui.spin_dscalemax->setValue(double(min_max_density.second) * m_scaling_actual2ui);
    }
}


void gmvis::ui::VisualizerWindow::on_cb_actual_x100_to_ui_scaling_toggled(bool checked)
{
    auto newScale = checked ? 100 : 1;
    if (m_scaling_actual2ui != newScale) {
        m_scaling_actual2ui = newScale;
        config.setValue("scaling_actual2ui_is_100", checked);
        QMessageBox::information(this, "setting scaling factor", "Please restart the application to activate the new setting.");
    }
}

void gmvis::ui::VisualizerWindow::on_cb_grey_bg_toggled(bool checked)
{
    ui.openGLWidget->setGreyBackground(checked);
    ui.openGLWidget->update();
}

void gmvis::ui::VisualizerWindow::on_btn_density_lock_symmetry_clicked()
{
    if (!ui.cb_dlog->isChecked()) {
        auto val_max = ui.spin_dscalemax->value() / m_scaling_actual2ui;
        auto val_min = ui.spin_dscalemin->value() / m_scaling_actual2ui;
        val_max = std::max(std::abs(val_min), val_max);
        ui.spin_dscalemax->setValue(val_max * m_scaling_actual2ui);
    }
}

void gmvis::ui::VisualizerWindow::on_spin_dscalemin_valueChanged(double arg1)
{
    if (ui.btn_density_lock_symmetry->isChecked()) {
        ui.spin_dscalemax->setValue(-arg1);
    }
}

void gmvis::ui::VisualizerWindow::on_spin_dscalemax_valueChanged(double arg1)
{
    if (ui.btn_density_lock_symmetry->isChecked()) {
        ui.spin_dscalemin->setValue(-arg1);
    }
}
