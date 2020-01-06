#include "GMDensityRenderer.h"
#include <math.h>

GMDensityRenderer::GMDensityRenderer(QOpenGLFunctions_4_5_Core* gl, DisplaySettings* settings, Camera* camera, int width, int height) : m_gl(gl), m_settings(settings), m_camera(camera) {
	m_program = std::make_unique<QOpenGLShaderProgram>();
	m_program->addShaderFromSourceFile(QOpenGLShader::Compute, "shaders/density.comp");
	m_program->link();

	m_program->bind();
	m_locOuttex = m_program->uniformLocation("outtex");
	m_locVolume = m_program->uniformLocation("density");
	m_locProjMatrix = m_program->uniformLocation("projMatrix");
	m_locViewMatrix = m_program->uniformLocation("viewMatrix");
	m_locWidth = m_program->uniformLocation("width");
	m_locHeight = m_program->uniformLocation("height");

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

	gl->glCreateFramebuffers(1, &m_fbo);
	gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_outtex, 0);
	gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	gl->glGenTextures(1, &m_volumetex);
	gl->glBindTexture(GL_TEXTURE_3D, m_volumetex);
	const GLfloat border[3] = { 0,0,0 };
	gl->glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, &border[0]);
	gl->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	gl->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	gl->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	gl->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl->glTexImage3D(GL_TEXTURE_3D, 0, GL_R16, 1, 1, 1, 0, GL_RED, GL_FLOAT, nullptr);
}

void GMDensityRenderer::setMixture(GaussianMixture* mixture)
{
	m_mixture = mixture;
	//Sample Volume
	//For now we just sample a fixed size Grid in 0.5-Steps. Later this should be chosen wiselier
	m_gl->glBindTexture(GL_TEXTURE_3D, m_volumetex);

	float samples[20 * 20 * 20];
	for (int i = 0; i < 20; ++i) {
		for (int j = 0; j < 20; ++j) {
			for (int k = 0; k < 20; ++k) {
				samples[i * 400 + j* 20 + k] = 1.0f;
				/*samples[i * 400 + j* 20 + k] = mixture->sample(double(i) / 2.0f - 5.0f, 
					double(j) / 2.0f - 5.0f, 
					double(k) / 2.0f - 5.0f);*/
			}
		}
	}

	m_gl->glTexImage3D(GL_TEXTURE_3D, 0, GL_R16, 20, 20, 20, 0, GL_RED, GL_FLOAT, &samples);
}

void GMDensityRenderer::setSize(int width, int height)
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

void GMDensityRenderer::render()
{
	if (!m_mixture) {
		return;
	}

	m_program->bind();
	m_gl->glActiveTexture(GL_TEXTURE0);
	m_gl->glBindImageTexture(0, m_outtex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	m_program->setUniformValue(m_locOuttex, 0);
	m_gl->glActiveTexture(GL_TEXTURE1);
	m_gl->glBindTexture(GL_TEXTURE_3D, m_volumetex);
	m_program->setUniformValue(m_locVolume, 1);
	m_program->setUniformValue(m_locWidth, m_screenWidth);
	m_program->setUniformValue(m_locHeight, m_screenHeight);
	m_gl->glDispatchCompute(ceil(m_screenWidth / 32.0f), ceil(m_screenHeight / 32.0), 1);
	m_gl->glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	m_gl->glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);

	m_gl->glBlitFramebuffer(0, 0, m_screenWidth, m_screenHeight, 0, 0, m_screenWidth, m_screenHeight,
		GL_COLOR_BUFFER_BIT, GL_LINEAR);


	//Problem: Wir überschreiben hier das Rendering der Punktwolke...
	//Aber für jetzt sollte das mal reichen, wir machens später.
	//TODO
}

void GMDensityRenderer::cleanup()
{
	m_gl->glDeleteFramebuffers(1, &m_fbo);
	m_gl->glDeleteTextures(1, &m_outtex);
	//ToDo: VolTex
	m_program.reset();
}


