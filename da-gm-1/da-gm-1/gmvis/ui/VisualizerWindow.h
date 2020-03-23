#pragma once

#include "gmvis/core/PointCloud.h"
#include "gmvis/core/DataLoader.h"
#include <memory>
#include <QtWidgets/QMainWindow>
#include "ui_VisualizerWindow.h"

namespace gmvis::ui {

	class VisualizerWindow : public QMainWindow
	{
		Q_OBJECT

	public:
		VisualizerWindow(QWidget* parent = Q_NULLPTR);

	public slots:
		void slotLoadPointcloud();
		void slotLoadMixture();
		void slotDisplayOptionsChanged();
		void slotEllValuesChanged();
		void slotEllAutoValueChanged();
		void slotEllipsoidRenderModeChanged();
		void slotDensityValuesChanged();
		void slotDensityAutoChanged();
		void slotDensityRenderModeChanged();
		void slotAccelerationThresholdChanged();
		void slotAccelerationThreshAutoChanged();
		void slotPostRender();

	private:
		Ui::VisualizerWindowClass ui;
		std::unique_ptr<core::PointCloud> pointcloud;
		std::unique_ptr<core::GaussianMixture> mixture;
	};
}