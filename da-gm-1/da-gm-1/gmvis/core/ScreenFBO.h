#pragma once
#include <QOpenGLFunctions_4_5_Core>

namespace gmvis::core {

	class ScreenFBO {

	public:
		ScreenFBO(QOpenGLFunctions_4_5_Core* gl, int width, int height, bool equalsize = false);	//equalsize: internal and actual size is the same
		void initialize();
		void attachSinglevalueTexture(GLuint attachment = 0);
		void attachColorTexture(GLuint attachment = 0);
		void attachDepthTexture();
		void setSize(int width, int height);
		GLuint getSinglevalueTexture();
		GLuint getColorTexture();
		GLuint getDepthTexture();
		int getWidth();
		int getHeight();
		int getID();
		void cleanup();

	private:
		QOpenGLFunctions_4_5_Core* m_gl;
		GLuint m_id;
		GLuint m_singleTex = -1;
		GLuint m_colorTex = -1;
		GLuint m_depthTex = -1;

		int m_fboWidth = 3000;
		int m_fboHeight = 3000;
		int m_screenWidth;
		int m_screenHeight;
		bool m_equalsize;
	};
}