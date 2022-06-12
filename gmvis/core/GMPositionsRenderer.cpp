#include "GMPositionsRenderer.h"
#include "DataLoader.h"
#include <QVector2D>

using namespace gmvis::core;

GMPositionsRenderer::GMPositionsRenderer(QOpenGLFunctions_4_5_Core* gl, Camera* camera) : m_gl(gl), m_camera(camera)
{
}

void GMPositionsRenderer::initialize()
{
	m_program = std::make_unique<QOpenGLShaderProgram>();
	m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, DataLoader::readRessource("shaders/gmpositions.vert"));
	m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, DataLoader::readRessource("shaders/gmpositions.frag"));
	m_program->link();

	m_program->bind();
	m_locProjMatrix = m_program->uniformLocation("projMatrix");
	m_locViewMatrix = m_program->uniformLocation("viewMatrix");
	m_locSurfaceColor = m_program->uniformLocation("surfaceColor");
	m_locTransferTex = m_program->uniformLocation("transferTex");
	m_locUseInColor = m_program->uniformLocation("useInColor");
	m_locMarkedGaussian = m_program->uniformLocation("markedGaussian");
	m_locWhiteMode = m_program->uniformLocation("whiteMode");

	m_program->release();

	//Create VAO
	m_gm_vao.create();
	m_gm_vao.bind();

	//Create Positions VBO
	m_pos_vbo.create();
	m_pos_vbo.bind();
	m_pos_vbo.allocate(nullptr, 0);
	m_gl->glEnableVertexAttribArray(0);
	m_gl->glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(QVector4D), 0);
	m_pos_vbo.release();

	//Create Colors VBO
	m_color_vbo.create();
	m_color_vbo.bind();
	m_color_vbo.allocate(nullptr, 0);
	m_gl->glEnableVertexAttribArray(1);
	m_gl->glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), 0);
	m_color_vbo.release();

	//Create Index VBO
	m_gindex_vbo.create();
	m_gindex_vbo.bind();
	m_gindex_vbo.allocate(nullptr, 0);
	m_gl->glEnableVertexAttribArray(2);
	m_gl->glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), 0);
	m_gindex_vbo.release();

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

void GMPositionsRenderer::setMixture(GaussianMixture<DECIMAL_TYPE>* mixture)
{
	m_mixture = mixture;
	m_markedGaussian = -1;

	m_gm_vao.bind();
	m_pos_vbo.bind();
	size_t arrsize = 0;
	auto data = mixture->gpuPositionData(arrsize, m_numberOfValidGaussians);
	m_pos_vbo.allocate(data.get(), arrsize);
	m_pos_vbo.release();
	m_gm_vao.release();

	QVector<GLfloat> indizes;
	indizes.reserve(mixture->numberOfGaussians());
	int i = mixture->nextEnabledGaussianIndex(-1);
	while (i != -1)
	{
		indizes.push_back(i);
		i = mixture->nextEnabledGaussianIndex(i);
	}
	m_gindex_vbo.bind();
	m_gindex_vbo.allocate(indizes.data(), indizes.size() * sizeof(GLfloat));
	m_gindex_vbo.release();

	updateColors();
}

void gmvis::core::GMPositionsRenderer::updateMixture()
{
	setMixture(m_mixture);
}

void GMPositionsRenderer::setUniformColor(const QColor& uniformColor)
{
	m_sUniformColor = uniformColor;
}

//updateColors needs to be called manually!!
void GMPositionsRenderer::setRenderMode(GMColoringRenderMode renderMode)
{
	m_sRenderMode = renderMode;
}

void GMPositionsRenderer::setEllMin(double min)
{
	m_sEllMin = min;
}

void GMPositionsRenderer::setEllMax(double max)
{
	m_sEllMax = max;
}

void GMPositionsRenderer::setRangeMode(GMColorRangeMode rangeMode)
{
	m_sRangeMode = rangeMode;
}

void gmvis::core::GMPositionsRenderer::setMarkedGaussian(int index)
{
	m_markedGaussian = index;
}

void gmvis::core::GMPositionsRenderer::setWhiteMode(bool white)
{
	m_whiteMode = white;
}

void GMPositionsRenderer::render()
{
	if (!m_mixture) {
		return;
	}

	GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	m_gl->glDrawBuffers(2, buffers);

	m_gm_vao.bind();
	m_program->bind();
	m_program->setUniformValue(m_locProjMatrix, m_camera->getProjMatrix());
	m_program->setUniformValue(m_locViewMatrix, m_camera->getViewMatrix());
	m_program->setUniformValue(m_locSurfaceColor, m_sUniformColor);
	m_program->setUniformValue(m_locUseInColor, (m_sRenderMode != GMColoringRenderMode::COLOR_UNIFORM));
	m_program->setUniformValue(m_locMarkedGaussian, m_markedGaussian);
	m_program->setUniformValue(m_locWhiteMode, m_whiteMode);
	m_gl->glActiveTexture(GL_TEXTURE0);
	m_gl->glBindTexture(GL_TEXTURE_1D, m_texTransfer);
	m_program->setUniformValue(m_locTransferTex, 0);
	m_gl->glPointSize(m_sPointSize);
	m_gl->glDrawArrays(GL_POINTS, 0, m_numberOfValidGaussians);
	m_program->release();
	m_gm_vao.release();
}

void GMPositionsRenderer::cleanup()
{
	m_pos_vbo.destroy();
	m_color_vbo.destroy();
	m_gindex_vbo.destroy();
	m_gm_vao.destroy();
	m_program.reset();
}

const QColor& GMPositionsRenderer::getUniformColor() const
{
	return m_sUniformColor;
}

const GMColoringRenderMode& GMPositionsRenderer::getRenderMode() const
{
	return m_sRenderMode;
}

const double& GMPositionsRenderer::getEllMin() const
{
	return m_sEllMin;
}

const double& GMPositionsRenderer::getEllMax() const
{
	return m_sEllMax;
}

const GMColorRangeMode& GMPositionsRenderer::getRangeMode() const
{
	return m_sRangeMode;
}

void GMPositionsRenderer::updateColors()
{
	if (!m_mixture) {
		return;
	}

	int n = m_mixture->numberOfGaussians();
	QVector<float> colors;
	colors.reserve(n);
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
			std::sort(values.begin(), values.end());
			double median = values[n / 2];
			QVector<double> deviations;
			deviations.resize(n);
			for (int i = 0; i < n; ++i) {
				float val = values[i];
				deviations[i] = abs(val - median);
			}
			std::sort(deviations.begin(), deviations.end());
			double medmed = deviations[n / 2];
			//Assign colors
			minVal = std::max(median - medmed, values[0]);
			maxVal = std::min(median + medmed, values[n - 1]);
		}
		double range = maxVal - minVal;
		int i = m_mixture->nextEnabledGaussianIndex(-1);
		while (i != -1) {
			const Gaussian<DECIMAL_TYPE>* gauss = (*m_mixture)[i];
			if (gauss->isValid()) {
				double val = (m_sRenderMode == GMColoringRenderMode::COLOR_WEIGHT) ? gauss->getNormalizedWeight() : gauss->getAmplitude();
				float t = std::min(1.0f, float((val - minVal) / range));
				t = std::max(t, 0.0f);
				colors.push_back(t);
			}
			else {
				colors.push_back(-1);
			}
			i = m_mixture->nextEnabledGaussianIndex(i);
		}
		m_sEllMin = minVal;
		m_sEllMax = maxVal;
	}
	else {
		int i = m_mixture->nextEnabledGaussianIndex(-1);
		while (i != -1) {
			const Gaussian<DECIMAL_TYPE>* gauss = (*m_mixture)[i];
			if (gauss->isValid()) {
				colors.push_back(1);
			}
			else {
				colors.push_back(-1);
			}
			i = m_mixture->nextEnabledGaussianIndex(i);
		}
	}

	m_color_vbo.bind();
	m_color_vbo.allocate(colors.data(), n * sizeof(float));
	m_color_vbo.release();
}