#pragma once
#include "GaussianMixture.h"
#include "DisplaySettings.h"
#include "Camera.h"
#include <QOpenGLFunctions_4_0_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

class GMIsoellipsoidRenderer {

public:
	GMIsoellipsoidRenderer(QOpenGLFunctions_4_0_Core* gl, DisplaySettings* settings, Camera* camera);
	void setMixture(GaussianMixture* mixture);
	void render();
	void cleanup();

private:
	QOpenGLFunctions_4_0_Core* m_gl;
	DisplaySettings* m_settings;
	Camera* m_camera;
	GaussianMixture* m_mixture = nullptr;

	//Geometry Data per Sphere
	QVector<QVector3D> m_geoVertices;
	QVector<QVector3D> m_geoNormals;
	QVector<GLuint> m_geoIndices;

	//Buffers
	QOpenGLVertexArrayObject m_gm_vao;
	QOpenGLBuffer m_pos_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	QOpenGLBuffer m_norm_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	QOpenGLBuffer m_indices_vbo = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
	QOpenGLBuffer m_transf_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	QOpenGLBuffer m_normtr_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);

	//Shader
	std::unique_ptr<QOpenGLShaderProgram> m_program;

	//Locations
	int m_projMatrixLoc;
	int m_viewMatrixLoc;
	int m_lightDirLoc;
	int m_surfaceColorLoc;
};