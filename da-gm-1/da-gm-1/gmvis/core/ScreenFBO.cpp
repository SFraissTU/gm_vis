#include "ScreenFBO.h"

using namespace gmvis::core;

ScreenFBO::ScreenFBO(QOpenGLFunctions_4_5_Core* gl, int width, int height, bool equalsize) : m_gl(gl), m_screenWidth(width), m_screenHeight(height), m_equalsize(equalsize)
{
	if (m_equalsize) {
		m_fboWidth = width;
		m_fboHeight = height;
	}
}

void ScreenFBO::initialize()
{
	m_gl->glCreateFramebuffers(1, &m_id);
}

void ScreenFBO::attachSinglevalueFloatTexture(GLuint attachment)
{
	m_gl->glGenTextures(1, &m_singleFloatTex);
	m_gl->glBindTexture(GL_TEXTURE_2D, m_singleFloatTex);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, m_fboWidth, m_fboHeight, 0, GL_RED, GL_FLOAT, nullptr);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_id);
	m_gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, GL_TEXTURE_2D, m_singleFloatTex, 0);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void gmvis::core::ScreenFBO::attachSinglevalueIntTexture(GLuint attachment)
{
	m_gl->glGenTextures(1, &m_singleIntTex);
	m_gl->glBindTexture(GL_TEXTURE_2D, m_singleIntTex);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, m_fboWidth, m_fboHeight, 0, GL_RED, GL_INT, nullptr);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_id);
	m_gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, GL_TEXTURE_2D, m_singleIntTex, 0);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ScreenFBO::attachColorTexture(GLuint attachment)
{
	m_gl->glGenTextures(1, &m_colorTex);
	m_gl->glBindTexture(GL_TEXTURE_2D, m_colorTex);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_fboWidth, m_fboHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_id);
	m_gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, GL_TEXTURE_2D, m_colorTex, 0);
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

void gmvis::core::ScreenFBO::attachStencilBuffer()
{
	m_gl->glGenRenderbuffers(1, &m_stencilTex);
	m_gl->glBindRenderbuffer(GL_RENDERBUFFER, m_stencilTex);
	m_gl->glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX16, m_fboWidth, m_fboHeight);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_id);
	m_gl->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_stencilTex);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_id);
	m_gl->glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void ScreenFBO::setSize(int width, int height)
{
	if (width > m_fboWidth || height > m_fboHeight || m_equalsize) {
		m_fboWidth = width;
		m_fboHeight = height;
		if (m_singleFloatTex != -1) {
			m_gl->glBindTexture(GL_TEXTURE_2D, m_singleFloatTex);
			m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, m_fboWidth, m_fboHeight, 0, GL_RED, GL_FLOAT, nullptr);
		}
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

GLuint ScreenFBO::getSinglevalueFloatTexture()
{
	return m_singleFloatTex;
}

GLuint gmvis::core::ScreenFBO::getSinglevalueIntTexture()
{
	return m_singleIntTex;
}

GLuint ScreenFBO::getColorTexture()
{
	return m_colorTex;
}

GLuint ScreenFBO::getDepthTexture()
{
	return m_depthTex;
}

GLuint gmvis::core::ScreenFBO::getStencilBuffer()
{
	return m_stencilTex;
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
	if (m_singleFloatTex != -1) {
		m_gl->glDeleteTextures(1, &m_singleFloatTex);
	}
	if (m_colorTex != -1) {
		m_gl->glDeleteTextures(1, &m_colorTex);
	}
	if (m_depthTex != -1) {
		m_gl->glDeleteTextures(1, &m_depthTex);
	}
}
