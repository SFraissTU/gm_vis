#include "GMIsoellipsoidRenderer.h"
#include <Eigen/Eigenvalues>

GMIsoellipsoidRenderer::GMIsoellipsoidRenderer(QOpenGLFunctions_4_0_Core* gl, DisplaySettings* settings, Camera* camera) : m_gl(gl), m_settings(settings), m_camera(camera)
{
	m_program = std::make_unique<QOpenGLShaderProgram>();
	m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "shaders/isoell.vert");
	m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "shaders/isoell.frag");
	m_program->link();

	m_program->bind();
	m_projMatrixLoc = m_program->uniformLocation("projMatrix");
	m_viewMatrixLoc = m_program->uniformLocation("viewMatrix");
	m_lightDirLoc = m_program->uniformLocation("lightDir");
	m_surfaceColorLoc = m_program->uniformLocation("surfaceColor");
	
	m_program->release();

	//Create VAO
	m_gm_vao.create();
	m_gm_vao.bind();

	//Create Positions VBO
	m_pos_vbo.create();
	m_pos_vbo.bind();
	m_pos_vbo.allocate(nullptr, 0);
	m_gl->glEnableVertexAttribArray(0);
	m_gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), 0);
	m_pos_vbo.release();

	//Create Normals VBO
	m_norm_vbo.create();
	m_norm_vbo.bind();
	m_norm_vbo.allocate(nullptr, 0);
	m_gl->glEnableVertexAttribArray(1);
	m_gl->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), 0);
	m_norm_vbo.release();

	//Create Transformations VBO (one transformation per instance)
	m_transf_vbo.create();
	m_transf_vbo.bind();
	m_transf_vbo.allocate(nullptr, 0);
	m_gl->glEnableVertexAttribArray(2);
	m_gl->glVertexAttribPointer(2, 16, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), 0);
	m_gl->glVertexAttribDivisor(2, 2);
	m_transf_vbo.release();

	//Create Normal-Transformations VBO (one transformation per instance)
	m_normtr_vbo.create();
	m_normtr_vbo.bind();
	m_normtr_vbo.allocate(nullptr, 0);
	m_gl->glEnableVertexAttribArray(3);
	m_gl->glVertexAttribPointer(3, 9, GL_FLOAT, GL_FALSE, sizeof(QMatrix3x3), 0);
	m_gl->glVertexAttribDivisor(2, 3);
	m_normtr_vbo.release();

	m_gm_vao.release();
}

void GMIsoellipsoidRenderer::setMixture(GaussianMixture* mixture)
{
	//Extract the Eigenvectors!
	
	int n = mixture->numberOfGaussians();
	for (int i = 0; i < n; ++i) {
		const Gaussian* gauss = (*mixture)[i];
		Eigen::Matrix3f cov = Eigen::Matrix3f();
		cov(0, 0) = gauss->covxx;
		cov(0, 1) = gauss->covxy;
		cov(0, 2) = gauss->covxz;
		cov(1, 0) = gauss->covxy;
		cov(1, 1) = gauss->covyy;
		cov(1, 2) = gauss->covyz;
		cov(2, 0) = gauss->covxz;
		cov(2, 1) = gauss->covyz;
		cov(2, 2) = gauss->covzz;
		Eigen::EigenSolver<Eigen::Matrix3Xf> eigensolver;
		eigensolver.compute(cov, true);
		//Get Eigenvectors and Eigenvalues and compute transformation matrix from there
	}

	m_gm_vao.bind();
	m_pos_vbo.bind();
}

void GMIsoellipsoidRenderer::render()
{
}

void GMIsoellipsoidRenderer::cleanup()
{
}
