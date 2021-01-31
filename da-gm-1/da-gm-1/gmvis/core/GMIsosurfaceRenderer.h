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
		void setMixture(GaussianMixture<DECIMAL_TYPE>* mixture, double accThreshold);
		void updateMixture(); //Call this when having changed the enabled gaussians settings in the mixture!
		void updateAccelerationData(double accThreshold);
		void render(int screenWidth, int screenHeight);
		void cleanup();

		void setIsolevel(float iso);
		const float& getIsolevel() const;

	private:
		QOpenGLFunctions_4_5_Core* m_gl;
		Camera* m_camera;
		GaussianMixture<DECIMAL_TYPE>* m_mixture = nullptr;

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
		GLuint m_proj_locImgStart;
		GLuint m_proj_locMaxFragListLen;
		GLuint m_proj_locInvViewMatrix;
		GLuint m_proj_locWidth;
		GLuint m_proj_locHeight;
		GLuint m_proj_locFov;
		GLuint m_rend_locInvViewMatrix;
		GLuint m_rend_locWidth;
		GLuint m_rend_locHeight;
		GLuint m_rend_locFov;
		GLuint m_rend_locGaussTex;
		GLuint m_rend_locIsolevel;
		GLuint m_rend_locImgStart;
		GLuint m_rend_locImgRendered;
		GLuint m_rend_locListSize;
		GLuint m_rend_locImgTest;

		GLuint m_ssboFragmentList;
		GLuint m_atomListSize;
		GLuint m_imgStartIdx;

		GLuint m_ssboMixture;
		GLuint m_texGauss;
		GLuint m_nrValidMixtureComponents;
		GLuint m_maxFragListLen = 10000000;

		ScreenFBO m_stencilFBO;
		ScreenFBO m_renderFBO;

		std::unique_ptr<QOpenGLShaderProgram> m_program_projection;
		std::unique_ptr<QOpenGLShaderProgram> m_program_sort_and_render_16;
		std::unique_ptr<QOpenGLShaderProgram> m_program_sort_and_render_32;
		std::unique_ptr<QOpenGLShaderProgram> m_program_sort_and_render_64;
		std::unique_ptr<QOpenGLShaderProgram> m_program_sort_and_render_128;

		float m_sIsolevel = 0.0001;
	};

}