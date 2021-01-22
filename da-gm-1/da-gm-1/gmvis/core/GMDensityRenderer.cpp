#include "GMDensityRenderer.h"
#include "DataLoader.h"

using namespace gmvis::core;

GMDensityRenderer::GMDensityRenderer(QOpenGLFunctions_4_5_Core* gl, Camera* camera, int width, int height) : 
	m_rasterizeRenderer(GMDensityRasterizeRenderer(gl, camera, width, height)),
	m_raycastRenderer(GMDensityRaycastRenderer(gl, camera, width, height)),
	m_fbo_intermediate(ScreenFBO(gl, width, height)),
	m_fbo_final(ScreenFBO(gl, width, height)),
	m_gl(gl)
{
	m_raycastRenderer.setAccelerationThreshold(m_sAccThreshold);
}

void GMDensityRenderer::initialize()
{
	m_fbo_intermediate.initialize();
	m_fbo_final.initialize();

	m_fbo_intermediate.attachSinglevalueFloatTexture();
	m_fbo_final.attachColorTexture();

	m_program_coloring = std::make_unique<QOpenGLShaderProgram>();
	m_program_coloring->addShaderFromSourceCode(QOpenGLShader::Compute, DataLoader::readRessource("shaders/density_coloring.comp"));
	m_program_coloring->link();

	m_program_coloring->bind();
	m_col_bindingOutimg = m_program_coloring->uniformLocation("img_output");
	m_col_bindingSumimg = m_program_coloring->uniformLocation("img_sum");
	m_col_bindingPreimg = m_program_coloring->uniformLocation("img_pre");
	m_col_locWidth = m_program_coloring->uniformLocation("width");
	m_col_locHeight = m_program_coloring->uniformLocation("height");
	m_col_locBlend = m_program_coloring->uniformLocation("blend");
	m_col_locTransferTex = m_program_coloring->uniformLocation("transferTex");
	m_col_locDensityMin = m_program_coloring->uniformLocation("densitymin");
	m_col_locDensityMax = m_program_coloring->uniformLocation("densitymax");
	m_col_locCutoff = m_program_coloring->uniformLocation("cutoff");
	m_col_locLogarithmic = m_program_coloring->uniformLocation("logarithmic");
	m_program_coloring->release();

	QVector<QVector3D> transferdata = DataLoader::readTransferFunction(QString("res/transfer.txt"));
	m_gl->glGenTextures(1, &m_texTransfer);
	m_gl->glBindTexture(GL_TEXTURE_1D, m_texTransfer);
	m_gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	m_gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	m_gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_gl->glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, transferdata.size(), 0, GL_RGB, GL_FLOAT, transferdata.data());
	
	m_rasterizeRenderer.initialize();
	m_raycastRenderer.initialize();
	setRenderMode(m_sRenderMode);
}

void GMDensityRenderer::setMixture(GaussianMixture<DECIMAL_TYPE>* mixture)
{
	m_mixture = mixture;
	m_rasterizeRenderer.setMixture(mixture, m_sAccThreshold);
	m_raycastRenderer.setMixture(mixture);
}

bool gmvis::core::GMDensityRenderer::hasMixture() const
{
	return m_mixture;
}

void GMDensityRenderer::setSize(int width, int height)
{
	m_fbo_intermediate.setSize(width, height);
	m_fbo_final.setSize(width, height);
	//m_rasterizeRenderer.setSize(width, height);
	//m_raycastRenderer.setSize(width, height);
}

void GMDensityRenderer::render(GLuint preTexture, bool blend)
{
	if (!m_mixture) {
		return;
	}

	GLint screenFbo = 0;
	m_gl->glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &screenFbo);
	m_gl->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo_intermediate.getID());
	int screenWidth = m_fbo_intermediate.getWidth();
	int screenHeight = m_fbo_intermediate.getHeight();

	if (m_sRenderMode == GMDensityRenderMode::ADDITIVE_ACC_PROJECTED) {
		m_rasterizeRenderer.render(screenWidth, screenHeight);
	}
	else {
		m_raycastRenderer.render(m_fbo_intermediate.getSinglevalueFloatTexture(), screenWidth, screenHeight);
	}

	//Auto-Density-Scale-Mode
	if (m_sDensityAuto) {
		m_gl->glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo_intermediate.getID());
		int nrpixels = screenWidth * screenHeight;
		float* pixeldata = new float[nrpixels];
		m_gl->glReadPixels(0, 0, screenWidth, screenHeight, GL_RED, GL_FLOAT, pixeldata);
		std::vector<float> pixels = std::vector<float>(pixeldata, pixeldata + nrpixels);
		std::sort(pixels.begin(), pixels.end());
		m_sDensityMin = 0;
		m_sDensityMax = pixels[(pixels.size() - 1) * m_sDensityAutoPerc];
		delete[] pixeldata;
	}

	m_program_coloring->bind();
	m_gl->glActiveTexture(GL_TEXTURE0);
	m_gl->glBindImageTexture(0, m_fbo_final.getColorTexture(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	m_program_coloring->setUniformValue(m_col_bindingOutimg, 0);
	m_gl->glActiveTexture(GL_TEXTURE1);
	m_gl->glBindImageTexture(1, m_fbo_intermediate.getSinglevalueFloatTexture(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	m_program_coloring->setUniformValue(m_col_bindingSumimg, 1);
	m_gl->glActiveTexture(GL_TEXTURE2);
	m_gl->glBindImageTexture(2, preTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	m_program_coloring->setUniformValue(m_col_bindingPreimg, 2);
	m_gl->glActiveTexture(GL_TEXTURE3);
	m_gl->glBindTexture(GL_TEXTURE_1D, m_texTransfer);
	m_program_coloring->setUniformValue(m_col_locTransferTex, 3);

	m_program_coloring->setUniformValue(m_col_locWidth, screenWidth);
	m_program_coloring->setUniformValue(m_col_locHeight, screenHeight);
	m_program_coloring->setUniformValue(m_col_locBlend, blend ? 0.5f : 1.0f);
	m_program_coloring->setUniformValue(m_col_locDensityMin, (float)(m_sLogarithmic ? m_sDensityMinLog : m_sDensityMin));
	m_program_coloring->setUniformValue(m_col_locDensityMax, (float)(m_sLogarithmic ? m_sDensityMaxLog : m_sDensityMax));
	m_program_coloring->setUniformValue(m_col_locCutoff, m_sDensityCutoff);
	m_program_coloring->setUniformValue(m_col_locLogarithmic, m_sLogarithmic);

	m_gl->glDispatchCompute(ceil(screenWidth / 32.0f), ceil(screenHeight / 32.0), 1);
	m_gl->glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	m_gl->glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo_final.getID());
	m_gl->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, screenFbo);

	m_gl->glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight,
		GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

void GMDensityRenderer::setRenderMode(GMDensityRenderMode mode)
{
	m_sRenderMode = mode;
	switch (m_sRenderMode) {
	case GMDensityRenderMode::ADDITIVE_EXACT:
		m_raycastRenderer.disableAccelerationStructure();
		break;
	case GMDensityRenderMode::ADDITIVE_SAMPLING_OCTREE:
		m_raycastRenderer.setSampling(true);
		m_raycastRenderer.enableAccelerationStructure();
		break;
	case GMDensityRenderMode::ADDITIVE_ACC_OCTREE:
		m_raycastRenderer.setSampling(false);
		m_raycastRenderer.enableAccelerationStructure();
		break;
	case GMDensityRenderMode::ADDITIVE_ACC_PROJECTED:
		m_raycastRenderer.disableAccelerationStructure();
		m_rasterizeRenderer.updateAccelerationData(m_sAccThreshold);
		break;
	default:
		qDebug() << "Unsupported Render Mode\n";
	}
}

void GMDensityRenderer::updateAccelerationData()
{
	if (m_sRenderMode == GMDensityRenderMode::ADDITIVE_ACC_OCTREE || m_sRenderMode == GMDensityRenderMode::ADDITIVE_SAMPLING_OCTREE) {
		m_raycastRenderer.rebuildAccelerationStructure();
	}
	else if (m_sRenderMode == GMDensityRenderMode::ADDITIVE_ACC_PROJECTED) {
		m_rasterizeRenderer.updateAccelerationData(m_sAccThreshold);
	}
}

void GMDensityRenderer::cleanup()
{
	m_fbo_intermediate.cleanup();
	m_fbo_final.cleanup();
	m_program_coloring.reset();
	m_rasterizeRenderer.cleanup();
	m_raycastRenderer.cleanup();
}

void GMDensityRenderer::setDensityMin(double densityMin)
{
	(m_sLogarithmic ? m_sDensityMinLog : m_sDensityMin) = densityMin;
}

void GMDensityRenderer::setDensityMax(double densityMax)
{
	(m_sLogarithmic ? m_sDensityMaxLog : m_sDensityMax) = densityMax;
	if (m_sAccThreshAuto) {
		m_sAccThreshold = std::max(m_sDensityMax * 0.0001, 0.000000000001);
	}
}

void gmvis::core::GMDensityRenderer::setDensityMinLog(double densityMinLog)
{
	m_sDensityMinLog = densityMinLog;
}

void gmvis::core::GMDensityRenderer::setDensityMaxLog(double densityMaxLog)
{
	m_sDensityMaxLog = densityMaxLog;
}

void GMDensityRenderer::setDensityAuto(bool densityAuto)
{
	m_sDensityAuto = densityAuto;
}

void GMDensityRenderer::setDensityAutoPercentage(double percentage)
{
	m_sDensityAutoPerc = percentage;
}

void GMDensityRenderer::setAccelerationThreshold(double accThreshold)
{
	m_sAccThreshold = accThreshold;
	m_raycastRenderer.setAccelerationThreshold(accThreshold);
}

void GMDensityRenderer::setAccelerationThresholdAuto(bool accThreshAuto)
{
	m_sAccThreshAuto = accThreshAuto;
	if (m_sAccThreshAuto) {
		m_sAccThreshold = m_sDensityMax * 0.0001;
	}
}

void gmvis::core::GMDensityRenderer::setDensityCutoff(bool cutoff)
{
	m_sDensityCutoff = cutoff;
}

void gmvis::core::GMDensityRenderer::setLogarithmic(bool log)
{
	m_sLogarithmic = log;
}

const GMDensityRenderMode& GMDensityRenderer::getRenderMode() const
{
	return m_sRenderMode;
}

const double& GMDensityRenderer::getDensityMin() const
{
	return m_sLogarithmic ? m_sDensityMinLog : m_sDensityMin;
}

const double& GMDensityRenderer::getDensityMax() const
{
	return m_sLogarithmic ? m_sDensityMaxLog : m_sDensityMax;
}

const bool& GMDensityRenderer::getDensityAuto() const
{
	return m_sDensityAuto;
}

const double& GMDensityRenderer::getDensityAutoPercentage() const
{
	return m_sDensityAutoPerc;
}

const double& GMDensityRenderer::getAccelerationThreshold() const
{
	return m_sAccThreshold;
}

const bool& GMDensityRenderer::getAccelerationThresholdAuto() const
{
	return m_sAccThreshAuto;
}

const bool& gmvis::core::GMDensityRenderer::getDensityCutoff() const
{
	return m_sDensityCutoff;
}

const bool& gmvis::core::GMDensityRenderer::getLogarithmic() const
{
	return m_sLogarithmic;
}

bool GMDensityRenderer::isAccelerated(GMDensityRenderMode mode)
{
	return mode == GMDensityRenderMode::ADDITIVE_ACC_OCTREE || mode == GMDensityRenderMode::ADDITIVE_ACC_PROJECTED || mode == GMDensityRenderMode::ADDITIVE_SAMPLING_OCTREE;
}
