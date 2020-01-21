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

class GMDensityRenderer {
public:
	GMDensityRenderer(QOpenGLFunctions_4_5_Core* gl, DisplaySettings* settings, Camera* camera, int width, int height);
	void setMixture(GaussianMixture* mixture);
	void setSize(int width, int height);
	void render(GLuint depthTexture);
	void cleanup();

private:
	QOpenGLFunctions_4_5_Core* m_gl;
	DisplaySettings* m_settings;
	Camera* m_camera;
	GaussianMixture* m_mixture = nullptr;

	ScreenFBO m_fbo;

	GLuint m_locOuttex;
	GLuint m_locMixture;
	GLuint m_locProjMatrix;
	GLuint m_locInvViewMatrix;
	GLuint m_locWidth;
	GLuint m_locHeight;
	GLuint m_locGaussTex;
	GLuint m_locTransferTex;
	GLuint m_locModelTex;
	GLuint m_locBlend;

	GLuint m_mixtureSsbo;
	GLuint m_gaussTexture;
	GLuint m_transferTexture;

	std::unique_ptr<QOpenGLShaderProgram> m_program;
};