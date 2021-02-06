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

	class GMIsoellipsoidRenderer {

	public:
		GMIsoellipsoidRenderer(QOpenGLFunctions_4_5_Core* gl, Camera* camera);
		void initialize();
		void setMixture(GaussianMixture<DECIMAL_TYPE>* mixture);
		void updateMixture(); //Call this when having changed the enabled gaussians settings in the mixture!
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
		void setWhiteMode(bool white);

        bool getDrawIsoEllipsoids() const;
        void setDrawIsoEllipsoids(bool drawIsoEllipsoids);

        double getIsoEllipsoidThreshold() const;
        void setIsoEllipsoidThreshold(double isoEllipsoidThreshold);

    private:
        QOpenGLFunctions_4_5_Core* m_gl;
        Camera* m_camera;
        GaussianMixture<DECIMAL_TYPE>* m_mixture = nullptr;

		//Geometry Data per Sphere
		QVector<QVector3D> m_geoVertices;
		QVector<QVector3D> m_geoNormals;
		QVector<GLuint> m_geoIndices;

		//Buffers
		QOpenGLVertexArrayObject m_gm_vao;
		QOpenGLBuffer m_pos_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		QOpenGLBuffer m_norm_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		QOpenGLBuffer m_indices_vbo = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
		QOpenGLBuffer m_transf_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		QOpenGLBuffer m_normtr_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		QOpenGLBuffer m_color_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		GLuint m_texTransfer;

		//Shader
		std::unique_ptr<QOpenGLShaderProgram> m_program;

		//Locations
		int m_locProjMatrix;
		int m_locViewMatrix;
		int m_locLightDir;
		int m_locUseInColor;
		int m_locSurfaceColor;
		int m_locTransferTex;
		int m_locEyePos;
		int m_locMarkedGaussian;
		int m_locWhiteMode;

		//Settings
		QColor					 m_sUniformColor = QColor(100, 100, 255);
		QVector3D				 m_sLightDirection = QVector3D(0.f, -0.7f, -1.0f).normalized();
		GMColoringRenderMode m_sRenderMode = GMColoringRenderMode::COLOR_UNIFORM;
		double m_sEllMin = 0;
		double m_sEllMax = 0.05;
		GMColorRangeMode m_sRangeMode = GMColorRangeMode::RANGE_MINMAX;
		bool m_sLightDirectionAuto = true;
		GLint m_numberOfValidGaussians;

		int m_markedGaussian = -1;
		bool m_whiteMode = false;

        bool m_drawIsoEllipsoids = false;
        double m_isoEllipsoidThreshold = 0.1;
	};
}
