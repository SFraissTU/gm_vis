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
#include <memory>

namespace gmvis::core {

	class GMDensityRasterizeRenderer {
	public:
		GMDensityRasterizeRenderer(QOpenGLFunctions_4_5_Core* gl, Camera* camera, int width, int height);
		void initialize();
		void setMixture(GaussianMixture<float>* mixture, double accThreshold);
		void updateAccelerationData(double accThreshold);
		void render(int screenWidth, int screenHeight);
		void cleanup();

	private:
		QOpenGLFunctions_4_5_Core* m_gl;
		Camera* m_camera;
		GaussianMixture<float>* m_mixture = nullptr;

		//Geometry Data per Sphere
		QVector<QVector3D> m_geoVertices;
		QVector<GLuint> m_geoIndices;

		//Geometry Buffers
		QOpenGLVertexArrayObject m_gm_vao;
		QOpenGLBuffer m_pos_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		QOpenGLBuffer m_indices_vbo = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
		QOpenGLBuffer m_transf_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);

		GLuint m_proj_locProjMatrix;
		GLuint m_proj_locViewMatrix;
		GLuint m_proj_locInvViewMatrix;
		GLuint m_proj_locWidth;
		GLuint m_proj_locHeight;
		GLuint m_proj_locFov;
		GLuint m_proj_locGaussTex;
		GLuint m_proj_bindingMixture;

		GLuint m_ssboMixture;
		GLuint m_texGauss;
		GLuint m_nrValidMixtureComponents;

		std::unique_ptr<QOpenGLShaderProgram> m_program_projection;
	};
}