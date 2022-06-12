#pragma once
#include <QOpenGLFunctions_4_5_Core>

namespace gmvis::core {

	class ScreenFBO {

	public:
		ScreenFBO(QOpenGLFunctions_4_5_Core* gl, int width, int height, bool equalsize = false);	//equalsize: internal and actual size is the same
		void initialize();
		void attachSinglevalueFloatTexture(GLuint attachment = 0);
		void attachSinglevalueIntTexture(GLuint attachment = 0);
		void attachColorTexture(GLuint attachment = 0);
		void attachDepthTexture();
		void attachStencilBuffer();
		void setSize(int width, int height);
		GLuint getSinglevalueFloatTexture();
		GLuint getSinglevalueIntTexture();
		GLuint getColorTexture();
		GLuint getDepthTexture();
		GLuint getStencilBuffer();
		int getWidth();
		int getHeight();
		int getID();
		void cleanup();

	private:
		QOpenGLFunctions_4_5_Core* m_gl;
		GLuint m_id;
		GLuint m_singleFloatTex = -1;
		GLuint m_singleIntTex = -1;
		GLuint m_colorTex = -1;
		GLuint m_depthTex = -1;
		GLuint m_stencilTex = -1;

		int m_fboWidth = 3000;
		int m_fboHeight = 3000;
		int m_screenWidth;
		int m_screenHeight;
		bool m_equalsize;
	};
}