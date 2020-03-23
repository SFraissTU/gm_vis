#pragma once
#include "PointCloud.h"
#include "Camera.h"
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QColor>

#include <memory>

namespace gmvis::core {

	class PointCloudRenderer {

	public:
		PointCloudRenderer(QOpenGLFunctions_4_5_Core* gl, Camera* camera);

		void initialize();

		void setPointCloud(PointCloud* pointCloud);
		void render();
		void cleanup();

		const QColor& getPointColor() const;
		const float& getPointSize() const;
		const bool& getPointCircles() const;

		void setPointColor(const QColor& pointColor);
		void setPointSize(float pointSize);
		void setPointCircles(bool pointCircles);

	private:
		QOpenGLFunctions_4_5_Core* m_gl;
		Camera* m_camera;
		PointCloud* m_pointcloud = nullptr;

		//Buffers
		QOpenGLVertexArrayObject m_pc_vao;
		QOpenGLBuffer m_pc_vbo;

		//Shader
		std::unique_ptr<QOpenGLShaderProgram> m_program;

		//Locations
		int m_locProjMatrix;
		int m_locViewMatrix;
		int m_colorLoc;
		int m_circlesLoc;

		//Settings
		QColor m_sPointColor = QColor(255, 255, 255);
		float  m_sPointSize = 1.0f;
		bool   m_sPointCircles = false;
	};
}