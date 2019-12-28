#pragma once

#include <memory>
#include <qopenglwidget.h>
#include <QOpenGLFunctions_4_0_Core>
#include <qopenglvertexarrayobject.h>
#include <qopenglbuffer.h>
#include <qopenglshaderprogram.h>
#include <qopengldebug.h>
#include <qevent.h>
#include "PointCloud.h"
#include "Camera.h"
#include "DisplaySettings.h"
#include "PointCloudRenderer.h"

class DisplayWidget : public QOpenGLWidget, private QOpenGLFunctions_4_0_Core {
	Q_OBJECT

public:
	DisplayWidget(QWidget* parent);
	~DisplayWidget();

	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;

	//should only be called after initialization!
	void setPointCloud(PointCloud* pointcloud);

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
	DisplaySettings m_settings;

	//Renderers
	std::unique_ptr<PointCloudRenderer> m_pointcloudRenderer;

	//Matrices (TODO)
	std::unique_ptr<Camera> m_camera;
	QPoint m_lastPos;

};
