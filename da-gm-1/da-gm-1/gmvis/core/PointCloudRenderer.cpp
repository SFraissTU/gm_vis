#include "PointCloudRenderer.h"

using namespace gmvis::core;

PointCloudRenderer::PointCloudRenderer(QOpenGLFunctions_4_5_Core* gl, Camera* camera) : m_gl(gl), m_camera(camera)
{
}

void PointCloudRenderer::initialize()
{
	m_program = std::make_unique<QOpenGLShaderProgram>();
	m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "shaders/pointcloud.vert");
	m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "shaders/pointcloud.frag");
	m_program->link();

	m_program->bind();
	m_locProjMatrix = m_program->uniformLocation("projMatrix");
	m_locViewMatrix = m_program->uniformLocation("viewMatrix");
	m_colorLoc = m_program->uniformLocation("pointcloudColor");
	m_circlesLoc = m_program->uniformLocation("circles");

	m_program->release();

	m_pc_vao.create();
	m_pc_vao.bind();
	m_pc_vbo.create();
	m_pc_vbo.bind();
	m_pc_vbo.allocate(nullptr, 0);
	m_gl->glEnableVertexAttribArray(0);
	m_gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(point_item), 0);
	m_pc_vbo.release();
	m_pc_vao.release();
}

void PointCloudRenderer::setPointCloud(PointCloud* pointcloud)
{
	m_pc_vao.bind();
	m_pc_vbo.bind();
	m_pc_vbo.allocate(pointcloud->getData(), pointcloud->getDataSize());
	m_gl->glEnableVertexAttribArray(0);
	m_gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, pointcloud->getSinglePointSize(), 0);
	m_pc_vbo.release();
	m_pc_vao.release();
	m_pointcloud = pointcloud;
}

void PointCloudRenderer::render()
{
	if (!m_pointcloud) {
		return;
	}

	m_pc_vao.bind();
	m_program->bind();
	m_program->setUniformValue(m_colorLoc, m_sPointColor);
	m_program->setUniformValue(m_circlesLoc, m_sPointCircles);
	m_program->setUniformValue(m_locProjMatrix, m_camera->getProjMatrix());
	m_program->setUniformValue(m_locViewMatrix, m_camera->getViewMatrix());
	m_gl->glPointSize(m_sPointSize);

	m_gl->glDrawArrays(GL_POINTS, 0, m_pointcloud->getPointCount());
	m_program->release();
	m_pc_vao.release();
}

void PointCloudRenderer::cleanup()
{
	m_pc_vbo.destroy();
	m_pc_vao.destroy();
	m_program.reset();
}

const QColor& PointCloudRenderer::getPointColor() const
{
	return m_sPointColor;
}

const float& PointCloudRenderer::getPointSize() const
{
	return m_sPointSize;
}

const bool& PointCloudRenderer::getPointCircles() const
{
	return m_sPointCircles;
}

void PointCloudRenderer::setPointColor(const QColor& pointColor)
{
	m_sPointColor = pointColor;
}

void PointCloudRenderer::setPointSize(float pointSize)
{
	m_sPointSize = pointSize;
}

void PointCloudRenderer::setPointCircles(bool pointCircles)
{
	m_sPointCircles = pointCircles;
}
