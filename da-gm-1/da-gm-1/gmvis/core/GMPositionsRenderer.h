#pragma once
#include "GaussianMixture.h"
#include "Camera.h"
#include "GMRenderModes.h"
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QColor>
#include <memory>

namespace gmvis::core {

	class GMPositionsRenderer {

	public:
		GMPositionsRenderer(QOpenGLFunctions_4_5_Core* gl, Camera* camera);
		void initialize();
		void setMixture(GaussianMixture<DECIMAL_TYPE>* mixture);
		void updateColors();
		void render();
		void cleanup();

		const QColor& getUniformColor() const;
		const GMColoringRenderMode& getRenderMode() const;
		const double& getEllMin() const;
		const double& getEllMax() const;
		const GMColorRangeMode& getRangeMode() const;

		void setUniformColor(const QColor& uniformColor);
		void setRenderMode(GMColoringRenderMode renderMode);
		void setEllMin(double min);
		void setEllMax(double max);
		void setRangeMode(GMColorRangeMode rangeMode);
		void setMarkedGaussian(int index);

	private:
		QOpenGLFunctions_4_5_Core* m_gl;
		Camera* m_camera;
		GaussianMixture<DECIMAL_TYPE>* m_mixture = nullptr;
		
		//Buffers
		QOpenGLVertexArrayObject m_gm_vao;
		QOpenGLBuffer m_pos_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		QOpenGLBuffer m_color_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		GLuint m_texTransfer;

		//Shader
		std::unique_ptr<QOpenGLShaderProgram> m_program;

		//Locations
		int m_locProjMatrix;
		int m_locViewMatrix;
		int m_locUseInColor;
		int m_locSurfaceColor;
		int m_locTransferTex;
		int m_locMarkedGaussian;

		//Settings
		QColor					 m_sUniformColor = QColor(100, 100, 255);
		GMColoringRenderMode m_sRenderMode = GMColoringRenderMode::COLOR_UNIFORM;
		double m_sEllMin = 0;
		double m_sEllMax = 0.05;
		GMColorRangeMode m_sRangeMode = GMColorRangeMode::RANGE_MINMAX;
		float  m_sPointSize = 5.0f;

		int m_markedGaussian = -1;
	};
}