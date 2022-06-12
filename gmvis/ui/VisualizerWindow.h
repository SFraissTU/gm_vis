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
        void slotEllipsoidCovOrIsoChanged();
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
		void slotHideZeroGaussians(bool checked);
		void slotHideInvalidGaussians(bool checked);
		void slotToggleBackground();
		void slotToggleFPS();

        void loadPureMixture(const QString& path);

    private slots:
        void on_btn_adams_scale_clicked();
        void on_cb_actual_x100_to_ui_scaling_toggled(bool checked);

        void on_cb_grey_bg_toggled(bool checked);

        void on_btn_density_lock_symmetry_clicked();

        void on_spin_dscalemin_valueChanged(double arg1);

        void on_spin_dscalemax_valueChanged(double arg1);

    private:
		Ui::VisualizerWindowClass ui;
		std::unique_ptr<core::PointCloud> pointcloud;
		std::unique_ptr<core::GaussianMixture<DECIMAL_TYPE>> mixture;
		std::unique_ptr<core::LineStrip> line;
		bool whiteMode = false;
		QSettings config;

		QString lineDirectory;

        double m_scaling_actual2ui = 1.0;

		void setNewMixture(std::unique_ptr<core::GaussianMixture<DECIMAL_TYPE>>& newGauss, const QString& fileLoadedFrom);
	};
}
