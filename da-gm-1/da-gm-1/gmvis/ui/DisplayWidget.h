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
#include "gmvis/core/Camera.h"
#include "gmvis/core/PointCloudRenderer.h"
#include "gmvis/core/GMIsoellipsoidRenderer.h"
#include "gmvis/core/GMDensityRenderer.h"

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
		void setDensityDisplayEnabled(bool enabled);

		bool isPointDisplayEnabled();
		bool isEllipsoidDisplayEnabled();
		bool isDensityDisplayEnabled();

		core::PointCloudRenderer* getPointCloudRenderer();
		core::GMIsoellipsoidRenderer* getGMIsoellipsoidRenderer();
		core::GMDensityRenderer* getGMDensityRenderer();

	public slots:
		void cleanup();

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
		bool m_sDisplayDensity = true;

		//Renderers
		std::unique_ptr<core::PointCloudRenderer> m_pointcloudRenderer;
		std::unique_ptr<core::GMIsoellipsoidRenderer> m_isoellipsoidRenderer;
		std::unique_ptr<core::GMDensityRenderer> m_densityRenderer;

		std::unique_ptr<core::ScreenFBO> m_fboIntermediate;

		//Matrices (TODO)
		std::unique_ptr<core::Camera> m_camera;
		QPoint m_lastPos;

	};
}