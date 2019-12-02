#pragma once
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
	std::vector<QVector3D> pointcloud;

	QOpenGLVertexArrayObject m_vao;
	QOpenGLBuffer m_vbo;
	std::unique_ptr<QOpenGLShaderProgram> m_program;
	std::unique_ptr<QOpenGLDebugLogger> m_debugLogger;
	int m_projMatrixLoc;
	int m_mvMatrixLoc;
	int m_lightPosLoc;

	QMatrix4x4 m_proj;
	QMatrix4x4 m_view;
	QMatrix4x4 m_world;

	float m_xRot = 180.0f;
	float m_yRot = 0;
	float m_zRot = 0;
	float m_radius = 40.0f;
	QPoint m_lastPos;

	static float normalizeAngle(float angle);
};