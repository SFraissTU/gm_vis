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
	void enableAccelerationStructure();
	void disableAccelerationStructure();
	void cleanup();

private:
	QOpenGLFunctions_4_5_Core* m_gl;
	DisplaySettings* m_settings;
	Camera* m_camera;
	GaussianMixture* m_mixture = nullptr;

	ScreenFBO m_fbo;

	GLuint m_locOuttex;
	GLuint m_locProjMatrix;
	GLuint m_locInvViewMatrix;
	GLuint m_locFov;
	GLuint m_locWidth;
	GLuint m_locHeight;
	GLuint m_locGaussTex;
	GLuint m_locTransferTex;
	GLuint m_locModelTex;
	GLuint m_locBlend;
	GLuint m_locDensityMin;
	GLuint m_locDensityMax;
	GLuint m_bindingMixture;
	GLuint m_bindingOctree;
	GLuint m_bindingTraversalMemory;

	GLuint m_ssboMixture;
	GLuint m_ssboOctree;
	GLuint m_ssboTraversalMemory;
	GLuint m_texGauss;
	GLuint m_texTransfer;

	bool validAccelerationStructure = false;
	bool useAccelerationStructure = true;

	std::unique_ptr<QOpenGLShaderProgram> m_program_regular;
	std::unique_ptr<QOpenGLShaderProgram> m_program_accelerated;

	void buildAccelerationStructure();
};