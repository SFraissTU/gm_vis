#pragma once

#include <memory>
#include <qopenglwidget.h>
#include <QOpenGLFunctions_4_5_Core>
#include <qopenglvertexarrayobject.h>
#include <qopenglbuffer.h>
#include <qopenglshaderprogram.h>
#include <qopengldebug.h>
#include <qevent.h>
#include "PointCloud.h"
#include "Camera.h"
#include "PointCloudRenderer.h"
#include "GMIsoellipsoidRenderer.h"
#include "GMDensityRenderer.h"

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

	PointCloudRenderer* getPointCloudRenderer();
	GMIsoellipsoidRenderer* getGMIsoellipsoidRenderer();
	GMDensityRenderer* getGMDensityRenderer();

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
	std::unique_ptr<PointCloudRenderer> m_pointcloudRenderer;
	std::unique_ptr<GMIsoellipsoidRenderer> m_isoellipsoidRenderer;
	std::unique_ptr<GMDensityRenderer> m_densityRenderer;

	std::unique_ptr<ScreenFBO> m_fboIntermediate;

	//Matrices (TODO)
	std::unique_ptr<Camera> m_camera;
	QPoint m_lastPos;

};