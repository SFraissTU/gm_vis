#pragma once
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include "Camera.h"
#include "GaussianMixture.h"
#include "ScreenFBO.h"

namespace gmvis::core {

	class GMIsosurfaceRenderer {
	public:
		GMIsosurfaceRenderer(QOpenGLFunctions_4_5_Core* gl, Camera* camera, int width, int height);
		void initialize();
		void setMixture(GaussianMixture* mixture, double accThreshold);
		void updateAccelerationData(double accThreshold);
		void render(int screenWidth, int screenHeight);
		void cleanup();

		void setIsolevel(float iso);
		const float& getIsolevel() const;

	private:
		QOpenGLFunctions_4_5_Core* m_gl;
		Camera* m_camera;
		GaussianMixture* m_mixture = nullptr;

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
		GLuint m_rend_locInvViewMatrix;
		GLuint m_rend_locWidth;
		GLuint m_rend_locHeight;
		GLuint m_rend_locFov;
		GLuint m_rend_locGaussTex;
		GLuint m_rend_locIsolevel;
		GLuint m_bindingMixture;

		GLuint m_ssboMixture;
		GLuint m_texGauss;
		GLuint m_nrValidMixtureComponents;

		ScreenFBO m_gaussianListFBO;

		std::unique_ptr<QOpenGLShaderProgram> m_program_projection;
		std::unique_ptr<QOpenGLShaderProgram> m_program_sort_and_render;

		float m_sIsolevel = 0.1;
	};

}