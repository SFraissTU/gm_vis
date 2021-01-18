#include "GMIsoellipsoidRenderer.h"
#include "Helper.h"
#include "DataLoader.h"

using namespace gmvis::core;

GMIsoellipsoidRenderer::GMIsoellipsoidRenderer(QOpenGLFunctions_4_5_Core* gl, Camera* camera) : m_gl(gl), m_camera(camera)
{
}

void GMIsoellipsoidRenderer::initialize()
{
	m_program = std::make_unique<QOpenGLShaderProgram>();
	m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, DataLoader::readRessource("shaders/ellipsoids.vert"));
	m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, DataLoader::readRessource("shaders/ellipsoids.frag"));
	m_program->link();

	m_program->bind();
	m_locProjMatrix = m_program->uniformLocation("projMatrix");
	m_locViewMatrix = m_program->uniformLocation("viewMatrix");
	m_locLightDir = m_program->uniformLocation("lightDir");
	m_locSurfaceColor = m_program->uniformLocation("surfaceColor");
	m_locTransferTex = m_program->uniformLocation("transferTex");
	m_locUseInColor = m_program->uniformLocation("useInColor");
	m_locEyePos = m_program->uniformLocation("eyePos");

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

	//Create Colors VBO (one color value per instance)
	m_color_vbo.create();
	m_color_vbo.bind();
	m_color_vbo.allocate(nullptr, 0);
	m_gl->glEnableVertexAttribArray(2);
	m_gl->glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float), 0);
	m_gl->glVertexAttribDivisor(2, 1);
	m_color_vbo.release();

	//Create Transformations VBO (one transformation per instance)
	m_transf_vbo.create();
	m_transf_vbo.bind();
	m_transf_vbo.allocate(nullptr, 0);
	m_gl->glEnableVertexAttribArray(3);
	m_gl->glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), 0);
	m_gl->glEnableVertexAttribArray(4);
	m_gl->glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), (void*)(sizeof(float) * 4));
	m_gl->glEnableVertexAttribArray(5);
	m_gl->glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), (void*)(sizeof(float) * 8));
	m_gl->glEnableVertexAttribArray(6);
	m_gl->glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), (void*)(sizeof(float) * 12));
	m_gl->glVertexAttribDivisor(3, 1);
	m_gl->glVertexAttribDivisor(4, 1);
	m_gl->glVertexAttribDivisor(5, 1);
	m_gl->glVertexAttribDivisor(6, 1);
	m_transf_vbo.release();

	//Create Normal-Transformations VBO (one transformation per instance)
	m_normtr_vbo.create();
	m_normtr_vbo.bind();
	m_normtr_vbo.allocate(nullptr, 0);
	m_gl->glEnableVertexAttribArray(7);
	m_gl->glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), 0);
	m_gl->glEnableVertexAttribArray(8);
	m_gl->glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), (void*)(sizeof(float) * 4));
	m_gl->glEnableVertexAttribArray(9);
	m_gl->glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), (void*)(sizeof(float) * 8));
	m_gl->glEnableVertexAttribArray(10);
	m_gl->glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), (void*)(sizeof(float) * 12));
	m_gl->glVertexAttribDivisor(7, 1);
	m_gl->glVertexAttribDivisor(8, 1);
	m_gl->glVertexAttribDivisor(9, 1);
	m_gl->glVertexAttribDivisor(10, 1);
	m_normtr_vbo.release();

	m_gm_vao.release();

	//Create Transfer Tex
	QVector<QVector3D> transferdata = DataLoader::readTransferFunction(QString("res/transfer.txt"));
	m_gl->glGenTextures(1, &m_texTransfer);
	m_gl->glBindTexture(GL_TEXTURE_1D, m_texTransfer);
	m_gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	m_gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	m_gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_gl->glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, transferdata.size(), 0, GL_RGB, GL_FLOAT, transferdata.data());
}

void GMIsoellipsoidRenderer::setMixture(GaussianMixture<DECIMAL_TYPE>* mixture)
{	
	m_mixture = mixture;
	int n = mixture->numberOfGaussians();
	QVector<QMatrix4x4> transforms;
	QVector<QMatrix4x4> normalTransfs;
	transforms.resize(n);
	normalTransfs.resize(n);
	for (int i = 0; i < n; ++i) {
		const Gaussian<DECIMAL_TYPE>* gauss = (*mixture)[i];

		auto eigenMatrix = gauss->getEigenMatrix();
		auto mu = gauss->getPosition();
		transforms[i] = QMatrix4x4(
			(float)eigenMatrix(0, 0), (float)eigenMatrix(0,1),  (float)eigenMatrix(0,2),  (float)mu.x(),
			(float)eigenMatrix(1, 0), (float)eigenMatrix(1,1),  (float)eigenMatrix(1, 2), (float)mu.y(),
			(float)eigenMatrix(2, 0), (float)eigenMatrix(2, 1), (float)eigenMatrix(2, 2), (float)mu.z(),
			0, 0, 0, 1
		);
		if (transforms[i].determinant() < 0) {
			//Mirror object so that determinant becomes positive
			//otherwise face ordering might switch
			transforms[i] = transforms[i] * QMatrix4x4(-1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1);
		}
		normalTransfs[i] = transforms[i].inverted().transposed();
	}

	m_transf_vbo.bind();
	m_transf_vbo.allocate(transforms.data(), n * sizeof(QMatrix4x4));
	m_transf_vbo.release();
	m_normtr_vbo.bind();
	m_normtr_vbo.allocate(normalTransfs.data(), n * sizeof(QMatrix4x4));
	m_normtr_vbo.release();

	updateColors();
}

void GMIsoellipsoidRenderer::setUniformColor(const QColor& uniformColor)
{
	m_sUniformColor = uniformColor;
}

//updateColors needs to be called manually!!
void GMIsoellipsoidRenderer::setRenderMode(GMColoringRenderMode renderMode)
{
	m_sRenderMode = renderMode;
}

void GMIsoellipsoidRenderer::setEllMin(double min)
{
	m_sEllMin = min;
}

void GMIsoellipsoidRenderer::setEllMax(double max)
{
	m_sEllMax = max;
}

void GMIsoellipsoidRenderer::setRangeMode(GMColorRangeMode rangeMode)
{
	m_sRangeMode = rangeMode;
}

void GMIsoellipsoidRenderer::render()
{
	if (!m_mixture) {
		return;
	}
	m_gm_vao.bind();
	m_program->bind();
	if (m_sLightDirectionAuto) {
		m_program->setUniformValue(m_locLightDir, m_camera->getViewDirection());
	}
	else {
		m_program->setUniformValue(m_locLightDir, m_sLightDirection);
	}
	m_program->setUniformValue(m_locProjMatrix, m_camera->getProjMatrix());
	m_program->setUniformValue(m_locViewMatrix, m_camera->getViewMatrix());
	m_program->setUniformValue(m_locEyePos, m_camera->getPosition());
	m_program->setUniformValue(m_locSurfaceColor, m_sUniformColor);
	m_program->setUniformValue(m_locUseInColor, (m_sRenderMode != GMColoringRenderMode::COLOR_UNIFORM));
	m_gl->glActiveTexture(GL_TEXTURE0);
	m_gl->glBindTexture(GL_TEXTURE_1D, m_texTransfer);
	m_program->setUniformValue(m_locTransferTex, 0);
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

const QColor& GMIsoellipsoidRenderer::getUniformColor() const
{
	return m_sUniformColor;
}

const GMColoringRenderMode& GMIsoellipsoidRenderer::getRenderMode() const
{
	return m_sRenderMode;
}

const double& GMIsoellipsoidRenderer::getEllMin() const
{
	return m_sEllMin;
}

const double& GMIsoellipsoidRenderer::getEllMax() const
{
	return m_sEllMax;
}

const GMColorRangeMode& GMIsoellipsoidRenderer::getRangeMode() const
{
	return m_sRangeMode;
}

void GMIsoellipsoidRenderer::updateColors()
{
	if (!m_mixture) {
		return;
	}

	int n = m_mixture->numberOfGaussians();
	QVector<DECIMAL_TYPE> colors;
	colors.resize(n);
	if (m_sRenderMode != GMColoringRenderMode::COLOR_UNIFORM) {
		//Find min and max Values
		double minVal = m_sEllMin;
		double maxVal = m_sEllMax;
		if (m_sRangeMode == GMColorRangeMode::RANGE_MINMAX) {
			minVal = std::numeric_limits<double>::infinity();
			maxVal = -minVal;
			for (int i = 0; i < n; ++i) {
				const Gaussian<DECIMAL_TYPE>* gauss = (*m_mixture)[i];
				double val = (m_sRenderMode == GMColoringRenderMode::COLOR_WEIGHT) ? gauss->getNormalizedWeight() : gauss->getAmplitude();
				if (val < minVal) {
					minVal = val;
				}
				if (val > maxVal) {
					maxVal = val;
				}
			}
		}
		else if (m_sRangeMode == GMColorRangeMode::RANGE_MEDMED) {
			double sum = 0;
			QVector<double> values;
			values.resize(n);
			for (int i = 0; i < n; ++i) {
				const Gaussian<DECIMAL_TYPE>* gauss = (*m_mixture)[i];
				double val = (m_sRenderMode == GMColoringRenderMode::COLOR_WEIGHT) ? gauss->getNormalizedWeight() : gauss->getAmplitude();
				values[i] = val;
			}
			qSort(values);
			double median = values[n / 2];
			QVector<double> deviations;
			deviations.resize(n);
			for (int i = 0; i < n; ++i) {
				float val = values[i];
				deviations[i] = abs(val - median);
			}
			qSort(deviations);
			double medmed = deviations[n / 2];
			//Assign colors
			minVal = std::max(median - medmed, values[0]);
			maxVal = std::min(median + medmed, values[n - 1]);
		}
		double range = maxVal - minVal;
		for (int i = 0; i < n; ++i) {
			const Gaussian<DECIMAL_TYPE>* gauss = (*m_mixture)[i];
			double val = (m_sRenderMode == GMColoringRenderMode::COLOR_WEIGHT) ? gauss->getNormalizedWeight() : gauss->getAmplitude();
			float t = std::min(1.0f, float((val - minVal) / range));
			t = std::max(t, 0.0f);
			colors[i] = t;
		}
		m_sEllMin = minVal;
		m_sEllMax = maxVal;
	}

	m_color_vbo.bind();
	m_color_vbo.allocate(colors.data(), n * sizeof(float));
	m_color_vbo.release();
}
