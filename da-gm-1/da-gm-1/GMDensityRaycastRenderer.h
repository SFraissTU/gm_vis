#pragma once
#include "GaussianMixture.h"
#include "Camera.h"
#include "ScreenFBO.h"
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLFramebufferObject>

class GMDensityRaycastRenderer {
public:
	GMDensityRaycastRenderer(QOpenGLFunctions_4_5_Core* gl, Camera* camera, int width, int height);
	void initialize();
	void setMixture(GaussianMixture* mixture);
	void setSize(int width, int height);
	void render(GLuint preTexture, bool blend, double densityMin, double densityMax);
	void enableAccelerationStructure();
	void disableAccelerationStructure();
	void rebuildAccelerationStructure();
	void setSampling(bool sampling);
	void setAccelerationStructureEnabled(bool enabled);
	void setAccelerationThreshold(double threshold);
	void cleanup();

private:
	QOpenGLFunctions_4_5_Core* m_gl;
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
	GLuint m_locPreImg;
	GLuint m_locBlend;
	GLuint m_locDensityMin;
	GLuint m_locDensityMax;
	GLuint m_bindingMixture;
	GLuint m_bindingOctree;

	GLuint m_ssboMixture;
	GLuint m_ssboOctree;
	GLuint m_texGauss;
	GLuint m_texTransfer;

	bool m_useSampling = false;
	bool m_validAccelerationStructure = false;
	bool m_useAccelerationStructure = true;

	std::unique_ptr<QOpenGLShaderProgram> m_program_regular;
	std::unique_ptr<QOpenGLShaderProgram> m_program_accelerated;
	std::unique_ptr<QOpenGLShaderProgram> m_program_sampling;

	double m_sAccThreshold;

	void buildAccelerationStructure();
	void buildUnacceleratedData();
};
