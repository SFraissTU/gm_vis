#pragma once
#include "PointCloud.h"
#include "DisplaySettings.h"
#include "Camera.h"
#include <QOpenGLFunctions_4_0_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

class PointCloudRenderer {

public:
	PointCloudRenderer(QOpenGLFunctions_4_0_Core* gl, DisplaySettings* settings, Camera* camera);
	void setPointCloud(PointCloud* pointCloud);
	void render();
	void cleanup();

private:
	QOpenGLFunctions_4_0_Core* m_gl;
	DisplaySettings* m_settings;
	Camera* m_camera;
	PointCloud* m_pointcloud = nullptr;

	//Buffers
	QOpenGLVertexArrayObject m_pc_vao;
	QOpenGLBuffer m_pc_vbo;

	//Shader
	std::unique_ptr<QOpenGLShaderProgram> m_program;

	//Locations
	int m_projMatrixLoc;
	int m_viewMatrixLoc;
	int m_lightPosLoc;
	int m_colorLoc;
	int m_circlesLoc;
};