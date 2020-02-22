#include "GMIsoellipsoidRenderer.h"
#include "Helper.h"

GMIsoellipsoidRenderer::GMIsoellipsoidRenderer(QOpenGLFunctions_4_5_Core* gl, DisplaySettings* settings, Camera* camera) : m_gl(gl), m_settings(settings), m_camera(camera)
{
	m_program = std::make_unique<QOpenGLShaderProgram>();
	m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "shaders/isoell.vert");
	m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "shaders/isoell.frag");
	m_program->link();

	m_program->bind();
	m_locProjMatrix = m_program->uniformLocation("projMatrix");
	m_locViewMatrix = m_program->uniformLocation("viewMatrix");
	m_lightDirLoc = m_program->uniformLocation("lightDir");
	m_surfaceColorLoc = m_program->uniformLocation("surfaceColor");
	
	m_program->release();

	//Create Geometry Data
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
	int n = mixture->numberOfGaussians();
	QVector<QMatrix4x4> transforms;
	QVector<QMatrix4x4> normalTransfs;
	transforms.resize(n);
	normalTransfs.resize(n);
	for (int i = 0; i < n; ++i) {
		const Gaussian* gauss = (*mixture)[i];

		auto eigenMatrix = gauss->getEigenMatrix();
		transforms[i] = QMatrix4x4(
			(float)eigenMatrix(0, 0), (float)eigenMatrix(0,1),  (float)eigenMatrix(0,2),  (float)gauss->mux,
			(float)eigenMatrix(1, 0), (float)eigenMatrix(1,1),  (float)eigenMatrix(1, 2), (float)gauss->muy,
			(float)eigenMatrix(2, 0), (float)eigenMatrix(2, 1), (float)eigenMatrix(2, 2), (float)gauss->muz,
			0, 0, 0, 1
		);
		if (transforms[i].determinant() < 0) {
			//Mirror object so that determinant becomes positive
			//otherwise face ordering might switch
			transforms[i] = transforms[i] * QMatrix4x4(-1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1);
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
	m_program->setUniformValue(m_locProjMatrix, m_camera->getProjMatrix());
	m_program->setUniformValue(m_locViewMatrix, m_camera->getViewMatrix());
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
