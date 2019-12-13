#pragma once
#include "PointCloud.h"
#include <qopenglwidget.h>
#include <qopenglfunctions.h>
#include <qopenglvertexarrayobject.h>
#include <qopenglbuffer.h>
#include <qopenglshaderprogram.h>
#include <qopengldebug.h>
#include <qevent.h>

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
	void wheelEvent(QWheelEvent* event) override;

protected slots:
	void messageLogged(const QOpenGLDebugMessage& msg);

private:
	//Debug Logger
	std::unique_ptr<QOpenGLDebugLogger> m_debugLogger;
	bool initialized = false;

	//Data to display
	PointCloud* pointcloud;

	//Buffers
	QOpenGLVertexArrayObject m_pc_vao;
	QOpenGLBuffer m_pc_vbo;

	//Shader
	std::unique_ptr<QOpenGLShaderProgram> m_program;

	//Locations
	int m_projMatrixLoc;
	int m_mvMatrixLoc;
	int m_lightPosLoc;

	//Matrices (TODO)
	QMatrix4x4 m_proj;
	QMatrix4x4 m_view;
	QMatrix4x4 m_world;

	//Control stuff
	float m_xRot = 180.0f;
	float m_yRot = 0;
	float m_zRot = 0;
	float m_radius = 40.0f;
	QPoint m_lastPos;

	//Help function: Moves angle to value between 0 and 360
	static float normalizeAngle(float angle);
};