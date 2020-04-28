#pragma once
#include "LineStrip.h"
#include "Camera.h"
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QColor>

#include <memory>

namespace gmvis::core {

	class LineRenderer {

	public:
		LineRenderer(QOpenGLFunctions_4_5_Core* gl, Camera* camera);

		void initialize();

		void setLineStrip(LineStrip* linestrip);
		void setMaxIteration(int iteration);
		void render(bool transparent);
		void cleanup();

	private:
		QOpenGLFunctions_4_5_Core* m_gl;
		Camera* m_camera;
		LineStrip* m_linestrip = nullptr;

		//Buffers
		QOpenGLVertexArrayObject m_ls_vao;
		QOpenGLBuffer m_ls_vbo;

		//Shader
		std::unique_ptr<QOpenGLShaderProgram> m_program;

		//Locations
		int m_locProjMatrix;
		int m_locViewMatrix;
		int m_opacityLoc;

		//Settings
		int m_maxiteration = -1; //-1 = none
	};
}