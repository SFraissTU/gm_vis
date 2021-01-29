#pragma once

#include "gmvis/core/PointCloud.h"
#include "gmvis/core/DataLoader.h"
#include <memory>
#include <QtWidgets/QMainWindow>
#include "ui_VisualizerWindow.h"
#include <QSettings>
#include "GaussianListItem.h"

namespace gmvis::ui {

	class VisualizerWindow : public QMainWindow
	{
		Q_OBJECT

	public:
		VisualizerWindow(QWidget* parent = Q_NULLPTR);

	public slots:
		void slotLoadPointcloud();
		void slotLoadMixtureModel();
		void slotLoadPureMixture();
		void slotLoadLine();
		void slotChooseLineDirectory();
		void slotGaussianSelected(int index);
		void slotDisplayOptionsChanged();
		void slotIsovalueDisplayOptionsChanged();
		void slotEllValuesChanged();
		void slotEllAutoValueChanged();
		void slotEllipsoidRenderModeChanged();
		void slotDensitySliderChanged();
		void slotDensityValuesChanged();
		void slotDensityAutoChanged();
		void slotDensityLogModeChanged();
		void slotDensityRenderModeChanged();
		void slotAccelerationThresholdChanged(bool update = true);
		void slotAccelerationThreshAutoChanged();
		void slotIsoValueChanged();
		void slotIsoSliderChanged();
		void slotPostRender();
		void slotResetDensity();
		void slotListGaussianSelected(QListWidgetItem* item);
		void slotTogglePickGaussian(bool checked);
		void slotClearSelection();
		void slotResetCamera();
		void slotCameraMoved(core::Camera* camera);

	private:
		Ui::VisualizerWindowClass ui;
		std::unique_ptr<core::PointCloud> pointcloud;
		std::unique_ptr<core::GaussianMixture<DECIMAL_TYPE>> mixture;
		std::unique_ptr<core::LineStrip> line;
		QSettings config;

		QString lineDirectory;

		void setNewMixture(std::unique_ptr<core::GaussianMixture<DECIMAL_TYPE>>& newGauss, const QString& fileLoadedFrom);
	};
}