#include "ScreenFBO.h"

ScreenFBO::ScreenFBO(QOpenGLFunctions_4_5_Core* gl, int width, int height, bool equalsize) : m_gl(gl), m_screenWidth(width), m_screenHeight(height), m_equalsize(equalsize)
{
	gl->glCreateFramebuffers(1, &m_id);

	if (m_equalsize) {
		m_fboWidth = width;
		m_fboHeight = height;
	}
}

void ScreenFBO::attachColorTexture()
{
	m_gl->glGenTextures(1, &m_colorTex);
	m_gl->glBindTexture(GL_TEXTURE_2D, m_colorTex);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_fboWidth, m_fboHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_id);
	m_gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTex, 0);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ScreenFBO::attachDepthTexture()
{
	m_gl->glGenTextures(1, &m_depthTex);
	m_gl->glBindTexture(GL_TEXTURE_2D, m_depthTex);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, m_fboWidth, m_fboHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_id);
	m_gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTex, 0);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ScreenFBO::setSize(int width, int height)
{
	if (width > m_fboWidth || height > m_fboHeight || m_equalsize) {
		m_fboWidth = std::max(m_fboWidth, width);
		m_fboHeight = std::max(m_fboHeight, height);
		if (m_colorTex != -1) {
			m_gl->glBindTexture(GL_TEXTURE_2D, m_colorTex);
			m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_fboWidth, m_fboHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
		}
		if (m_depthTex != -1) {
			m_gl->glBindTexture(GL_TEXTURE_2D, m_depthTex);
			m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_fboWidth, m_fboHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		}
	}
	m_screenWidth = width;
	m_screenHeight = height;
}

GLuint ScreenFBO::getColorTexture()
{
	return m_colorTex;
}

GLuint ScreenFBO::getDepthTexture()
{
	return m_depthTex;
}

int ScreenFBO::getWidth()
{
	return m_screenWidth;
}

int ScreenFBO::getHeight()
{
	return m_screenHeight;
}

int ScreenFBO::getID()
{
	return m_id;
}

void ScreenFBO::cleanup()
{
	m_gl->glDeleteFramebuffers(1, &m_id);
	if (m_colorTex != -1) {
		m_gl->glDeleteTextures(1, &m_colorTex);
	}
	if (m_depthTex != -1) {
		m_gl->glDeleteTextures(1, &m_depthTex);
	}
}
