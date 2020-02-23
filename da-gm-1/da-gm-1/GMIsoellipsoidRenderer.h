#pragma once
#include "GaussianMixture.h"
#include "DisplaySettings.h"
#include "Camera.h"
#include "GMRenderModes.h"
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <memory>

class GMIsoellipsoidRenderer {

public:
	GMIsoellipsoidRenderer(QOpenGLFunctions_4_5_Core* gl, DisplaySettings* settings, Camera* camera, GMIsoellipsoidRenderMode renderMode);
	void setMixture(GaussianMixture* mixture);
	void setRenderMode(GMIsoellipsoidRenderMode renderMode);
	void render();
	void cleanup();

private:
	QOpenGLFunctions_4_5_Core* m_gl;
	DisplaySettings* m_settings;
	Camera* m_camera;
	GaussianMixture* m_mixture = nullptr;
	GMIsoellipsoidRenderMode m_renderMode;

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
	QOpenGLBuffer m_color_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);

	//Shader
	std::unique_ptr<QOpenGLShaderProgram> m_program;

	//Locations
	int m_locProjMatrix;
	int m_locViewMatrix;
	int m_lightDirLoc;

	void updateColors();
};
