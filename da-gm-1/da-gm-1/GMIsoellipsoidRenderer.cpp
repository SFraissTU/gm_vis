#include "GMIsoellipsoidRenderer.h"
#include "Helper.h"
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

	//Create Geometry Data
	//Helper::createTriangle(30.0f, m_geoVertices, m_geoNormals, m_geoIndices);
	Helper::createSphere(1.0f, 32.0f, 32.0f, m_geoVertices, m_geoNormals, m_geoIndices);

	//Create VAO
	m_gm_vao.create();
	m_gm_vao.bind();

	//Create Positions VBO
	m_pos_vbo.create();
	m_pos_vbo.bind();
	m_pos_vbo.allocate(m_geoVertices.data(), m_geoVertices.size() * sizeof(QVector3D));
	m_gl->glEnableVertexAttribArray(0);
	m_gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), 0);
	m_pos_vbo.release();

	//Create Normals VBO
	m_norm_vbo.create();
	m_norm_vbo.bind();
	m_norm_vbo.allocate(m_geoNormals.data(), m_geoNormals.size() * sizeof(QVector3D));
	m_gl->glEnableVertexAttribArray(1);
	m_gl->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), 0);
	m_norm_vbo.release();

	//Create Indices VBO
	m_indices_vbo.create();
	m_indices_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	m_indices_vbo.bind();
	m_indices_vbo.allocate(m_geoIndices.data(), m_geoIndices.size() * sizeof(GLuint));

	//Create Transformations VBO (one transformation per instance)
	m_transf_vbo.create();
	m_transf_vbo.bind();
	m_transf_vbo.allocate(nullptr, 0);
	m_gl->glEnableVertexAttribArray(2);
	m_gl->glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), 0);
	m_gl->glEnableVertexAttribArray(3);
	m_gl->glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), (void*)(sizeof(float) * 4));
	m_gl->glEnableVertexAttribArray(4);
	m_gl->glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), (void*)(sizeof(float) * 8));
	m_gl->glEnableVertexAttribArray(5);
	m_gl->glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), (void*)(sizeof(float) * 12));
	m_gl->glVertexAttribDivisor(2, 1);
	m_gl->glVertexAttribDivisor(3, 1);
	m_gl->glVertexAttribDivisor(4, 1);
	m_gl->glVertexAttribDivisor(5, 1);
	m_transf_vbo.release();

	//Create Normal-Transformations VBO (one transformation per instance)
	m_normtr_vbo.create();
	m_normtr_vbo.bind();
	m_normtr_vbo.allocate(nullptr, 0);
	m_gl->glEnableVertexAttribArray(6);
	m_gl->glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), 0);
	m_gl->glEnableVertexAttribArray(7);
	m_gl->glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), (void*)(sizeof(float) * 4));
	m_gl->glEnableVertexAttribArray(8);
	m_gl->glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), (void*)(sizeof(float) * 8));
	m_gl->glEnableVertexAttribArray(9);
	m_gl->glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), (void*)(sizeof(float) * 12));
	m_gl->glVertexAttribDivisor(6, 1);
	m_gl->glVertexAttribDivisor(7, 1);
	m_gl->glVertexAttribDivisor(8, 1);
	m_gl->glVertexAttribDivisor(9, 1);
	m_normtr_vbo.release();

	m_gm_vao.release();
}

void GMIsoellipsoidRenderer::setMixture(GaussianMixture* mixture)
{
	//ToDo: Extract the Eigenvectors!
	
	int n = mixture->numberOfGaussians();
	QVector<QMatrix4x4> transforms;
	QVector<QMatrix4x4> normalTransfs;
	transforms.resize(n);
	normalTransfs.resize(n);
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
		Eigen::VectorXf eigen_values = eigensolver.eigenvalues().real();
		Eigen::MatrixXf eigen_vectors = eigensolver.eigenvectors().real();
		float l0 = sqrt(eigen_values(0));
		float l1 = sqrt(eigen_values(1));
		float l2 = sqrt(eigen_values(2));
		transforms[i] = QMatrix4x4(
			l0 * eigen_vectors(0, 0), l1 * eigen_vectors(0, 1), l2 * eigen_vectors(0, 2), gauss->x,
			l0 * eigen_vectors(1, 0), l1 * eigen_vectors(1, 1), l2 * eigen_vectors(1, 2), gauss->y,
			l0 * eigen_vectors(2, 0), l1 * eigen_vectors(2, 1), l2 * eigen_vectors(2, 2), gauss->z,
			0, 0, 0, 1
		);
		if (transforms[i].determinant() < 0) {
			//Switch two rows so that determinant becomes positive
			//otherwise face ordering might switch
			transforms[i] = transforms[i] * QMatrix4x4(-1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1);
			//QVector4D temprow = transforms[i].row(0);
			//transforms[i].setRow(0, transforms[i].row(1));
			//transforms[i].setRow(1, temprow);
			//transforms[i](0, 3) = gauss->x;	//last column has to stay the same. shouldn't affect det.
			//transforms[i](1, 3) = gauss->y;
		}
		//transforms[i].translate(QVector3D(gauss->x, gauss->y, gauss->z));
		normalTransfs[i] = transforms[i].inverted().transposed();
	}

	m_transf_vbo.bind();
	m_transf_vbo.allocate(transforms.data(), n * sizeof(QMatrix4x4));
	m_transf_vbo.release();
	m_normtr_vbo.bind();
	m_normtr_vbo.allocate(normalTransfs.data(), n * sizeof(QMatrix4x4));
	m_normtr_vbo.release();
	m_mixture = mixture;
}

void GMIsoellipsoidRenderer::render()
{
	if (!m_mixture) {
		return;
	}
	m_gm_vao.bind();
	m_program->bind();
	m_program->setUniformValue(m_surfaceColorLoc, m_settings->ellipsoidColor);
	m_program->setUniformValue(m_lightDirLoc, m_settings->lightDirection);
	m_program->setUniformValue(m_projMatrixLoc, m_camera->getProjMatrix());
	m_program->setUniformValue(m_viewMatrixLoc, m_camera->getViewMatrix());
	//m_gl->glDrawArraysInstanced(GL_TRIANGLES, 0, m_geoVertices.count(), 3);
	//m_gl->glDrawArrays(GL_TRIANGLES, 0, m_geoVertices.count());
	//m_gl->glDrawElements(GL_TRIANGLES, m_geoIndices.count(), GL_UNSIGNED_INT, nullptr);
	m_gl->glDrawElementsInstanced(GL_TRIANGLES, m_geoIndices.count(), GL_UNSIGNED_INT, nullptr, m_mixture->numberOfGaussians());
	m_program->release();
	m_gm_vao.release();
}

void GMIsoellipsoidRenderer::cleanup()
{
	m_pos_vbo.destroy();
	m_norm_vbo.destroy();
	m_transf_vbo.destroy();
	m_normtr_vbo.destroy();
	m_program.reset();
}
