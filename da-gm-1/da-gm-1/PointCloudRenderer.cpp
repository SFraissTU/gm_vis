#include "PointCloudRenderer.h"

PointCloudRenderer::PointCloudRenderer(QOpenGLFunctions_4_0_Core* gl, DisplaySettings* settings, Camera* camera) : m_gl(gl), m_settings(settings), m_camera(camera)
{
	m_program = std::make_unique<QOpenGLShaderProgram>();
	m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "shaders/pointcloud.vert");
	m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "shaders/pointcloud.frag");
	//m_program->bindAttributeLocation("in_vertex", 0);
	m_program->link();

	m_program->bind();
	m_projMatrixLoc = m_program->uniformLocation("projMatrix");
	m_viewMatrixLoc = m_program->uniformLocation("viewMatrix");
	//m_lightPosLoc = m_program->uniformLocation("lightPos");
	m_colorLoc = m_program->uniformLocation("pointcloudColor");
	m_circlesLoc = m_program->uniformLocation("circles");

	//m_program->setUniformValue(m_lightPosLoc, QVector3D(0, 0, 2));

	m_program->release();

	m_pc_vao.create();
	m_pc_vbo.create();
	m_pc_vao.bind();
	m_pc_vbo.bind();
	m_pc_vbo.allocate(nullptr, 0);
	m_gl->glEnableVertexAttribArray(0);
	m_gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
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
	/*if (m_pc_vao == -1) {
		return;
	}*/

	m_pc_vao.bind();
	m_program->bind();
	m_program->setUniformValue(m_colorLoc, m_settings->pointcloudColor);
	m_program->setUniformValue(m_circlesLoc, m_settings->circles);
	m_program->setUniformValue(m_projMatrixLoc, m_camera->getProjMatrix());
	m_program->setUniformValue(m_viewMatrixLoc, m_camera->getViewMatrix());
	m_gl->glPointSize(m_settings->pointSize);

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
