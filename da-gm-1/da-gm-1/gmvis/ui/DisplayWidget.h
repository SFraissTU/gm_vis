#pragma once

#include <memory>
#include <qopenglwidget.h>
#include <QOpenGLFunctions_4_5_Core>
#include <qopenglvertexarrayobject.h>
#include <qopenglbuffer.h>
#include <qopenglshaderprogram.h>
#include <qopengldebug.h>
#include <qevent.h>
#include "gmvis/core/PointCloud.h"
#include "gmvis/core/GaussianMixture.h"
#include "gmvis/core/Camera.h"
#include "gmvis/core/PointCloudRenderer.h"
#include "gmvis/core/GMIsoellipsoidRenderer.h"
#include "gmvis/core/GMDensityRenderer.h"
#include "gmvis/core/GMPositionsRenderer.h"
#include "gmvis/core/GMIsosurfaceRenderer.h"
#include "gmvis/core/LineRenderer.h"

namespace gmvis::ui {

	class DisplayWidget : public QOpenGLWidget, private QOpenGLFunctions_4_5_Core {
		Q_OBJECT

	public:
		DisplayWidget(QWidget* parent);
		~DisplayWidget();

		QSize minimumSizeHint() const override;
		QSize sizeHint() const override;

		void setPointDisplayEnabled(bool enabled);
		void setEllipsoidDisplayEnabled(bool enabled);
		void setGMPositionsDisplayEnabled(bool enabled);
		void setIsosurfaceDisplayEnabled(bool enabled);
		void setDensityDisplayEnabled(bool enabled);
		void setPickingEnabled(bool enabled);
		
		bool isPointDisplayEnabled() const;
		bool isEllipsoidDisplayEnabled() const;
		bool isGMPositionsDisplayEnabled() const;
		bool isDensityDisplayEnabled() const;
		bool isIsosurfaceDisplayEnabled() const;
		bool isGMVisibleInAnyWay() const;

		core::PointCloudRenderer* getPointCloudRenderer();
		core::GMIsoellipsoidRenderer* getGMIsoellipsoidRenderer();
		core::GMPositionsRenderer* getGMPositionsRenderer();
		core::GMIsosurfaceRenderer* getGMIsosurfaceRenderer();
		core::GMDensityRenderer* getGMDensityRenderer();
		core::LineRenderer* getLineRenderer();

		core::Camera* getCamera();
		void setMixture(gmvis::core::GaussianMixture<DECIMAL_TYPE>* mixture);

	public slots:
		void cleanup();

	signals:
		void gaussianSelected(int index);
		void cameraMoved(core::Camera* camera);

	protected:
		void initializeGL() override;
		void paintGL() override;
		void resizeGL(int width, int height) override;
		void mousePressEvent(QMouseEvent* event) override;
		void mouseMoveEvent(QMouseEvent* event) override;

	protected slots:
		void messageLogged(const QOpenGLDebugMessage& msg);

	private:
		//Debug Logger
		std::unique_ptr<QOpenGLDebugLogger> m_debugLogger;

		//Settings
		bool m_sDisplayPoints = false;
		bool m_sDisplayEllipsoids = false;
		bool m_sDisplayGMPositions = false;
		bool m_sDisplayIsosurface = false;
		bool m_sDisplayDensity = true;
		bool m_picking = false;

		//Renderers
		std::unique_ptr<core::PointCloudRenderer> m_pointcloudRenderer;
		std::unique_ptr<core::GMIsoellipsoidRenderer> m_isoellipsoidRenderer;
		std::unique_ptr<core::GMPositionsRenderer> m_positionsRenderer;
		std::unique_ptr<core::GMDensityRenderer> m_densityRenderer;
		std::unique_ptr<core::GMIsosurfaceRenderer> m_isosurfaceRenderer;
		std::unique_ptr<core::LineRenderer> m_lineRenderer;

		std::unique_ptr<core::ScreenFBO> m_fboIntermediate;

		core::GaussianMixture<DECIMAL_TYPE>* m_mixture = nullptr;

		//Matrices (TODO)
		std::unique_ptr<core::Camera> m_camera;
		QPoint m_lastPos;

	};
}