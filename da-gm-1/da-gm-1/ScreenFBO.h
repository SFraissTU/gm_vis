#pragma once
#include <QOpenGLFunctions_4_5_Core>

class ScreenFBO {

public:
	ScreenFBO(QOpenGLFunctions_4_5_Core* gl, int width, int height, bool equalsize);	//equalsize: internal and actual size is the same
	void attachColorTexture();
	void attachDepthTexture();
	void setSize(int width, int height);
	GLuint getColorTexture();
	GLuint getDepthTexture();
	int getWidth();
	int getHeight();
	int getID();
	void cleanup();

private:
	QOpenGLFunctions_4_5_Core* m_gl;
	GLuint m_id;
	GLuint m_colorTex = -1;
	GLuint m_depthTex = -1;

	int m_fboWidth = 3000;
	int m_fboHeight = 3000;
	int m_screenWidth;
	int m_screenHeight;
	bool m_equalsize;
};