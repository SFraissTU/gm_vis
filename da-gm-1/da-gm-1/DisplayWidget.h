#pragma once

#include <memory>
#include <qopenglwidget.h>
#include <qopenglfunctions.h>
#include <qopenglvertexarrayobject.h>
#include <qopenglbuffer.h>
#include <qopenglshaderprogram.h>
#include <qopengldebug.h>
#include <qevent.h>
#include "PointCloud.h"
#include "Camera.h"

class DisplayWidget : public QOpenGLWidget, private QOpenGLFunctions {
	Q_OBJECT

public:
	DisplayWidget(QWidget* parent = nullptr);
	~DisplayWidget();

	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;

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
	bool m_initialized = false;

	//Data to display
	PointCloud* m_pointcloud;

	//Buffers
	QOpenGLVertexArrayObject m_pc_vao;
	QOpenGLBuffer m_pc_vbo;

	//Shader
	std::unique_ptr<QOpenGLShaderProgram> m_program;

	//Locations
	int m_projMatrixLoc;
	int m_viewMatrixLoc;
	int m_lightPosLoc;

	//Matrices (TODO)
	std::unique_ptr<Camera> m_camera;
	QPoint m_lastPos;

};
