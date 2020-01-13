#pragma once
#include "GaussianMixture.h"
#include "DisplaySettings.h"
#include "Camera.h"
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLFrameBufferObject>

class GMDensityRendererDirectSampled {
public:
	GMDensityRendererDirectSampled(QOpenGLFunctions_4_5_Core* gl, DisplaySettings* settings, Camera* camera, int width, int height);
	void setMixture(GaussianMixture* mixture);
	void setSize(int width, int height);
	void render();
	void cleanup();

private:
	QOpenGLFunctions_4_5_Core* m_gl;
	DisplaySettings* m_settings;
	Camera* m_camera;
	GaussianMixture* m_mixture = nullptr;

	int m_fboWidth = 3000;
	int m_fboHeight = 3000;
	int m_screenWidth;
	int m_screenHeight;

	GLuint m_locOuttex;
	GLuint m_locMixture;
	GLuint m_locProjMatrix;
	GLuint m_locViewMatrix;
	GLuint m_locInvViewMatrix;
	GLuint m_locWidth;
	GLuint m_locHeight;

	GLuint m_mixtureSsbo;

	GLuint m_outtex;
	GLuint m_fbo;

	std::unique_ptr<QOpenGLShaderProgram> m_program;
};