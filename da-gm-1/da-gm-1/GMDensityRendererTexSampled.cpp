#include "GMDensityRendererTexSampled.h"
#include <math.h>

GMDensityRendererTexSampled::GMDensityRendererTexSampled(QOpenGLFunctions_4_5_Core* gl, DisplaySettings* settings, Camera* camera, int width, int height) : m_gl(gl), m_settings(settings), m_camera(camera) {
	m_program = std::make_unique<QOpenGLShaderProgram>();
	m_program->addShaderFromSourceFile(QOpenGLShader::Compute, "shaders/density.comp");
	m_program->link();

	m_program->bind();
	m_locOuttex = m_program->uniformLocation("outtex");
	m_locVolume = m_program->uniformLocation("density");
	m_locVolExtent = m_program->uniformLocation("volextent");
	m_locProjMatrix = m_program->uniformLocation("projMatrix");
	m_locViewMatrix = m_program->uniformLocation("viewMatrix");
	m_locInvViewMatrix = m_program->uniformLocation("invViewMatrix");
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

void GMDensityRendererTexSampled::setMixture(GaussianMixture* mixture)
{
	m_mixture = mixture;
	//Sample Volume
	//For now we just sample a fixed size Grid in 0.5-Steps. Later this should be chosen wiselier
	m_gl->glBindTexture(GL_TEXTURE_3D, m_volumetex);

	//k = x
	//j = y
	//i = z
	float max = 0;
	float* samples = new float[m_sampleaxiscount * m_sampleaxiscount * m_sampleaxiscount];
	for (int i = 0; i < m_sampleaxiscount; ++i) {
		for (int j = 0; j < m_sampleaxiscount; ++j) {
			for (int k = 0; k < m_sampleaxiscount; ++k) {
				//samples[i * 400 + j * 20 + k] = 1.0f;// (float(k) / 20.0f)* (float(i) / 20.0f)* (float(j) / 20.0f);
				float val = mixture->sample(m_volumeextent * (double(k)/m_sampleaxiscount - 0.5f),
					m_volumeextent * (double(j) / m_sampleaxiscount - 0.5f),
					m_volumeextent * (double(i) / m_sampleaxiscount - 0.5f));
				//float val = 0.03 - std::sqrt(std::pow(k-10,2) + std::pow(j-10,2) + std::pow(i-10,2)) / 1000.0;
				samples[i * m_sampleaxiscount*m_sampleaxiscount + j * m_sampleaxiscount + k] = val;
				if (val > max) max = val;
			}
		}
	}

	m_gl->glTexImage3D(GL_TEXTURE_3D, 0, GL_R16, m_sampleaxiscount, m_sampleaxiscount, m_sampleaxiscount, 0, GL_RED, GL_FLOAT, samples);
}

void GMDensityRendererTexSampled::setSize(int width, int height)
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

void GMDensityRendererTexSampled::render()
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
	m_program->setUniformValue(m_locVolExtent, m_volumeextent);
	m_program->setUniformValue(m_locWidth, m_screenWidth);
	m_program->setUniformValue(m_locHeight, m_screenHeight);
	m_program->setUniformValue(m_locInvViewMatrix, m_camera->getViewMatrix().inverted());
	m_gl->glDispatchCompute(ceil(m_screenWidth / 32.0f), ceil(m_screenHeight / 32.0), 1);
	m_gl->glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	m_gl->glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);

	m_gl->glBlitFramebuffer(0, 0, m_screenWidth, m_screenHeight, 0, 0, m_screenWidth, m_screenHeight,
		GL_COLOR_BUFFER_BIT, GL_LINEAR);


	//Problem: Wir �berschreiben hier das Rendering der Punktwolke...
	//Aber f�r jetzt sollte das mal reichen, wir machens sp�ter.
	//TODO
}

void GMDensityRendererTexSampled::cleanup()
{
	m_gl->glDeleteFramebuffers(1, &m_fbo);
	m_gl->glDeleteTextures(1, &m_outtex);
	//ToDo: VolTex
	m_program.reset();
}


