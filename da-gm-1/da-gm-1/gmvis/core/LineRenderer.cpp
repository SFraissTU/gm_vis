#include "LineRenderer.h"

using namespace gmvis::core;

LineRenderer::LineRenderer(QOpenGLFunctions_4_5_Core* gl, Camera* camera) : m_gl(gl), m_camera(camera)
{
}

void LineRenderer::initialize()
{
	m_program = std::make_unique<QOpenGLShaderProgram>();
	m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "shaders/linestrip.vert");
	m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "shaders/linestrip.frag");
	m_program->link();

	m_program->bind();
	m_locProjMatrix = m_program->uniformLocation("projMatrix");
	m_locViewMatrix = m_program->uniformLocation("viewMatrix");
	m_opacityLoc = m_program->uniformLocation("opacity");

	m_program->release();

	m_ls_vao.create();
	m_ls_vao.bind();
	m_ls_vbo.create();
	m_ls_vbo.bind();
	m_ls_vbo.allocate(nullptr, 0);
	m_gl->glEnableVertexAttribArray(0);
	m_gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(point_item), 0);
	m_ls_vbo.release();
	m_ls_vao.release();
}

void LineRenderer::setLineStrip(LineStrip* linestrip)
{
	if (linestrip) {
		m_ls_vao.bind();
		m_ls_vbo.bind();
		m_ls_vbo.allocate(linestrip->getData(), linestrip->getDataSize());
		m_gl->glEnableVertexAttribArray(0);
		m_gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, linestrip->getSinglePointSize(), 0);
		m_ls_vbo.release();
		m_ls_vao.release();
	}
	m_linestrip = linestrip;
	if (m_maxiteration > m_linestrip->getPointCount()) {
		qDebug() << "Not enough points in line strip!\n";
	}
}

void LineRenderer::setMaxIteration(int iteration)
{
	m_maxiteration = iteration;
}

void LineRenderer::render(bool transparent)
{
	if (!m_linestrip) {
		return;
	}
	if (m_linestrip->getPointCount() == 0) {
		qDebug() << "Error: empty line strip!" << "\n";
		return;
	}

	m_ls_vao.bind();
	m_program->bind();

	m_gl->glEnable(GL_BLEND);
	m_gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	m_program->setUniformValue(m_opacityLoc, transparent ? 0.5f : 1.0f);
	m_program->setUniformValue(m_locProjMatrix, m_camera->getProjMatrix());
	m_program->setUniformValue(m_locViewMatrix, m_camera->getViewMatrix());

	m_gl->glDrawArrays(GL_LINE_STRIP, 0, (m_maxiteration == -1) ? m_linestrip->getPointCount() : m_maxiteration);
	m_program->release();
	m_ls_vao.release();

	m_gl->glDisable(GL_BLEND);
}

void LineRenderer::cleanup()
{
	m_ls_vbo.destroy();
	m_ls_vao.destroy();
	m_program.reset();
}
