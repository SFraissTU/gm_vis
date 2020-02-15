#pragma once
#include "GaussianMixture.h"
#include "DisplaySettings.h"
#include "Camera.h"
#include "ScreenFBO.h"
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLFrameBufferObject>

class GMDensityRendererAcc2 {
public:
	GMDensityRendererAcc2(QOpenGLFunctions_4_5_Core* gl, DisplaySettings* settings, Camera* camera, int width, int height);
	void setMixture(GaussianMixture* mixture);
	void setSize(int width, int height);
	void render(GLuint preTexture);
	void cleanup();

private:
	QOpenGLFunctions_4_5_Core* m_gl;
	DisplaySettings* m_settings;
	Camera* m_camera;
	GaussianMixture* m_mixture = nullptr;

	//Geometry Data per Sphere
	QVector<QVector3D> m_geoVertices;
	QVector<GLuint> m_geoIndices;

	//Geometry Buffers
	QOpenGLVertexArrayObject m_gm_vao;
	QOpenGLBuffer m_pos_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	QOpenGLBuffer m_indices_vbo = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
	QOpenGLBuffer m_transf_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	
	ScreenFBO m_fbo_projection;
	ScreenFBO m_fbo_final;

	GLuint m_proj_locProjMatrix;
	GLuint m_proj_locViewMatrix;
	GLuint m_proj_locInvViewMatrix;
	GLuint m_proj_locWidth;
	GLuint m_proj_locHeight;
	GLuint m_proj_locFov;
	GLuint m_proj_locGaussTex;
	GLuint m_proj_bindingMixture;

	GLuint m_col_bindingOutimg;
	GLuint m_col_bindingSumimg;
	GLuint m_col_bindingPreimg;
	GLuint m_col_locWidth;
	GLuint m_col_locHeight;
	GLuint m_col_locBlend;
	GLuint m_col_locTransferTex;
	GLuint m_col_locDensityMin;
	GLuint m_col_locDensityMax;

	GLuint m_ssboMixture;
	GLuint m_texGauss;
	GLuint m_texTransfer;
	GLuint m_nrValidMixtureComponents;

	std::unique_ptr<QOpenGLShaderProgram> m_program_projection;
	std::unique_ptr<QOpenGLShaderProgram> m_program_coloring;
};