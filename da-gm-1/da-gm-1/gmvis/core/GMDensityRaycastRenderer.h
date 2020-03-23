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

namespace gmvis::core {

	class GMDensityRaycastRenderer {
	public:
		GMDensityRaycastRenderer(QOpenGLFunctions_4_5_Core* gl, Camera* camera, int width, int height);
		void initialize();
		void setMixture(GaussianMixture* mixture);
		void render(GLuint outTexture, int screenWidth, int screenHeight);
		void enableAccelerationStructure();
		void disableAccelerationStructure();
		void rebuildAccelerationStructure();
		void setSampling(bool sampling);
		void setAccelerationStructureEnabled(bool enabled);
		void setAccelerationThreshold(double threshold);
		void cleanup();

	private:
		QOpenGLFunctions_4_5_Core* m_gl;
		Camera* m_camera;
		GaussianMixture* m_mixture = nullptr;

		GLuint m_locOuttex;
		GLuint m_locInvViewMatrix;
		GLuint m_locFov;
		GLuint m_locWidth;
		GLuint m_locHeight;
		GLuint m_locGaussTex;
		GLuint m_bindingMixture;
		GLuint m_bindingOctree;

		GLuint m_ssboMixture;
		GLuint m_ssboOctree;
		GLuint m_texGauss;

		bool m_useSampling = false;
		bool m_validAccelerationStructure = false;
		bool m_useAccelerationStructure = true;

		std::unique_ptr<QOpenGLShaderProgram> m_program_regular;
		std::unique_ptr<QOpenGLShaderProgram> m_program_accelerated;
		std::unique_ptr<QOpenGLShaderProgram> m_program_sampling;

		double m_sAccThreshold;

		void buildAccelerationStructure();
		void buildUnacceleratedData();
	};
}