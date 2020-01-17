#include "GMDensityRendererAnalyticAdd.h"
#include "DataLoader.h"
#include <math.h>

GMDensityRendererAnalyticAdd::GMDensityRendererAnalyticAdd(QOpenGLFunctions_4_5_Core* gl, DisplaySettings* settings, Camera* camera, int width, int height) : m_gl(gl), m_settings(settings), m_camera(camera) {
	m_program = std::make_unique<QOpenGLShaderProgram>();
	m_program->addShaderFromSourceFile(QOpenGLShader::Compute, "shaders/density_analyticadd.comp");
	m_program->link();

	m_program->bind();
	m_locOuttex = m_program->uniformLocation("outtex");
	m_locMixture = 0;
	m_locProjMatrix = m_program->uniformLocation("projMatrix");
	m_locViewMatrix = m_program->uniformLocation("viewMatrix");
	m_locInvViewMatrix = m_program->uniformLocation("invViewMatrix");
	m_locWidth = m_program->uniformLocation("width");
	m_locHeight = m_program->uniformLocation("height");
	m_locGaussTex = m_program->uniformLocation("gaussTex");
	m_locTransferTex = m_program->uniformLocation("transferTex");

	m_program->release();

	m_screenWidth = width;
	m_screenHeight = height;

	gl->glGenTextures(1, &m_outtex);
	gl->glBindTexture(GL_TEXTURE_2D, m_outtex);
	gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_fboWidth, m_fboHeight, 0, GL_RGBA, GL_FLOAT, nullptr);

	double factor = 1.0 / sqrt(0.5);
	float* gaussdata = new float[1001];
	gaussdata[0] = 0;
	gaussdata[1000] = 1;
	for (int i = 1; i < 1000; ++i) {
		double t = (i - 500) / 100.0;
		gaussdata[i] = 0.5 * erfc(-t / factor);
	}
	gl->glGenTextures(1, &m_gaussTexture);
	gl->glBindTexture(GL_TEXTURE_1D, m_gaussTexture);
	gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl->glTexImage1D(GL_TEXTURE_1D, 0, GL_RED, 1001, 0, GL_RED, GL_FLOAT, gaussdata);
	delete gaussdata;
	
	QVector<QVector3D> transferdata = DataLoader::readTransferFunction(QString("res/transfer.txt"));
	gl->glGenTextures(1, &m_transferTexture);
	gl->glBindTexture(GL_TEXTURE_1D, m_transferTexture);
	gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl->glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, transferdata.size(), 0, GL_RGB, GL_FLOAT, transferdata.data());

	gl->glCreateFramebuffers(1, &m_fbo);
	gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_outtex, 0);
	gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);

	gl->glCreateBuffers(1, &m_mixtureSsbo);
}

void GMDensityRendererAnalyticAdd::setMixture(GaussianMixture* mixture)
{
	m_mixture = mixture;
	size_t arrsize;
	std::shared_ptr<char[]> gpudata = mixture->gpuData(arrsize);

	m_gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_mixtureSsbo);
	m_gl->glBufferData(GL_SHADER_STORAGE_BUFFER, arrsize, gpudata.get(), GL_STATIC_DRAW);
	m_gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void GMDensityRendererAnalyticAdd::setSize(int width, int height)
{
	if (width > m_fboWidth || height > m_fboHeight) {
		m_fboWidth = std::max(m_fboWidth, width);
		m_fboHeight = std::max(m_fboHeight, height);
		m_gl->glBindTexture(GL_TEXTURE_2D, m_outtex);
		m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_fboWidth, m_fboHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
	}
	m_screenWidth = width;
	m_screenHeight = height;
}

void GMDensityRendererAnalyticAdd::render()
{
	if (!m_mixture) {
		return;
	}

	m_program->bind();
	m_gl->glActiveTexture(GL_TEXTURE0);
	m_gl->glBindImageTexture(0, m_outtex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	m_program->setUniformValue(m_locOuttex, 0);
	m_gl->glActiveTexture(GL_TEXTURE1);
	m_gl->glBindTexture(GL_TEXTURE_1D, m_gaussTexture);
	m_program->setUniformValue(m_locGaussTex, 1);
	m_gl->glActiveTexture(GL_TEXTURE2);
	m_gl->glBindTexture(GL_TEXTURE_1D, m_transferTexture);
	m_program->setUniformValue(m_locTransferTex, 2);

	m_gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_mixtureSsbo);
	m_gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_locMixture, m_mixtureSsbo);
	m_gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	m_program->setUniformValue(m_locWidth, m_screenWidth);
	m_program->setUniformValue(m_locHeight, m_screenHeight);
	m_program->setUniformValue(m_locInvViewMatrix, m_camera->getViewMatrix().inverted());
	m_gl->glDispatchCompute(ceil(m_screenWidth / 32.0f), ceil(m_screenHeight / 32.0), 1);
	m_gl->glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	m_gl->glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);

	m_gl->glBlitFramebuffer(0, 0, m_screenWidth, m_screenHeight, 0, 0, m_screenWidth, m_screenHeight,
		GL_COLOR_BUFFER_BIT, GL_LINEAR);


	//Problem: Wir überschreiben hier das Rendering der Punktwolke...
	//Aber für jetzt sollte das mal reichen, wir machens später.
	//TODO
}

void GMDensityRendererAnalyticAdd::cleanup()
{
	m_gl->glDeleteFramebuffers(1, &m_fbo);
	m_gl->glDeleteTextures(1, &m_outtex);
	//ToDo: VolTex
	m_program.reset();
}


