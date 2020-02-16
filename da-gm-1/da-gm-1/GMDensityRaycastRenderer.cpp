#include "GMDensityRaycastRenderer.h"
#include "DataLoader.h"
#include <math.h>
#include <QtMath>

GMDensityRaycastRenderer::GMDensityRaycastRenderer(QOpenGLFunctions_4_5_Core* gl, DisplaySettings* settings, Camera* camera, int width, int height) : m_gl(gl), m_settings(settings), m_camera(camera), m_fbo(ScreenFBO(gl, width, height, false)) {
	m_program_regular = std::make_unique<QOpenGLShaderProgram>();
	m_program_regular->addShaderFromSourceFile(QOpenGLShader::Compute, "shaders/density.comp");
	m_program_regular->link();

	m_program_accelerated = std::make_unique<QOpenGLShaderProgram>();
	m_program_accelerated->addShaderFromSourceFile(QOpenGLShader::Compute, "shaders/density_acs.comp");
	m_program_accelerated->link();

	m_program_accelerated->bind();
	//Locations should be the same in both programs
	m_locOuttex = m_program_regular->uniformLocation("outtex");
	m_bindingMixture = 0;
	m_bindingOctree = 1;
	m_locProjMatrix = m_program_regular->uniformLocation("projMatrix");
	m_locInvViewMatrix = m_program_regular->uniformLocation("invViewMatrix");
	m_locWidth = m_program_regular->uniformLocation("width");
	m_locHeight = m_program_regular->uniformLocation("height");
	m_locFov = m_program_regular->uniformLocation("fov");
	m_locGaussTex = m_program_regular->uniformLocation("gaussTex");
	m_locTransferTex = m_program_regular->uniformLocation("transferTex");
	m_locModelTex = m_program_regular->uniformLocation("modelTex");
	m_locBlend = m_program_regular->uniformLocation("blend");
	m_locDensityMin = m_program_regular->uniformLocation("densitymin");
	m_locDensityMax = m_program_regular->uniformLocation("densitymax");

	m_program_regular->release();

	m_fbo.attachColorTexture();

	double factor = 1.0 / sqrt(0.5);
	float* gaussdata = new float[1001];
	gaussdata[0] = 0;
	gaussdata[1000] = 1;
	for (int i = 1; i < 1000; ++i) {
		double t = (i - 500) / 100.0;
		gaussdata[i] = 0.5 * erfc(-t / factor);
	}
	gl->glGenTextures(1, &m_texGauss);
	gl->glBindTexture(GL_TEXTURE_1D, m_texGauss);
	gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl->glTexImage1D(GL_TEXTURE_1D, 0, GL_RED, 1001, 0, GL_RED, GL_FLOAT, gaussdata);
	delete[] gaussdata;
	
	QVector<QVector3D> transferdata = DataLoader::readTransferFunction(QString("res/transfer.txt"));
	gl->glGenTextures(1, &m_texTransfer);
	gl->glBindTexture(GL_TEXTURE_1D, m_texTransfer);
	gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl->glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, transferdata.size(), 0, GL_RGB, GL_FLOAT, transferdata.data());
	
	gl->glCreateBuffers(1, &m_ssboMixture);
	gl->glCreateBuffers(1, &m_ssboOctree);
}

void GMDensityRaycastRenderer::setMixture(GaussianMixture* mixture)
{
	m_mixture = mixture;

	if (!useAccelerationStructure) {
		buildUnacceleratedData();
	}
	else {
		buildAccelerationStructure();
	}
}

void GMDensityRaycastRenderer::setSize(int width, int height)
{
	m_fbo.setSize(width, height);
}

void GMDensityRaycastRenderer::render(GLuint depthTexture)
{
	if (!m_mixture) {
		return;
	}

	/*m_gl->glBindFramebuffer(GL_READ_FRAMEBUFFER, depthTexture);

	m_gl->glBlitFramebuffer(0, 0, m_fbo.getWidth(), m_fbo.getHeight(), 0, 0, m_fbo.getWidth(), m_fbo.getHeight(),
		GL_COLOR_BUFFER_BIT, GL_LINEAR);
	return;*/

	auto& currentProgram = useAccelerationStructure ? m_program_accelerated : m_program_regular;
	
	currentProgram->bind();
	m_gl->glActiveTexture(GL_TEXTURE0);
	m_gl->glBindImageTexture(0, m_fbo.getColorTexture(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	currentProgram->setUniformValue(m_locOuttex, 0);
	m_gl->glActiveTexture(GL_TEXTURE1);
	m_gl->glBindTexture(GL_TEXTURE_1D, m_texGauss);
	currentProgram->setUniformValue(m_locGaussTex, 1);
	m_gl->glActiveTexture(GL_TEXTURE2);
	m_gl->glBindTexture(GL_TEXTURE_1D, m_texTransfer);
	currentProgram->setUniformValue(m_locTransferTex, 2);
	m_gl->glActiveTexture(GL_TEXTURE3);
	m_gl->glBindTexture(GL_TEXTURE_2D, depthTexture);
	currentProgram->setUniformValue(m_locModelTex, 3);

	m_gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboMixture);
	m_gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bindingMixture, m_ssboMixture);

	if (useAccelerationStructure) {
		m_gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboOctree);
		m_gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bindingOctree, m_ssboOctree);
	}

	m_gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	int screenWidth = m_fbo.getWidth();
	int screenHeight = m_fbo.getHeight();
	m_program_regular->setUniformValue(m_locWidth, screenWidth);
	m_program_regular->setUniformValue(m_locHeight, screenHeight);
	m_program_regular->setUniformValue(m_locInvViewMatrix, m_camera->getViewMatrix().inverted());
	m_program_regular->setUniformValue(m_locFov, qDegreesToRadians(m_camera->getFoV()));
	m_program_regular->setUniformValue(m_locBlend, (m_settings->displayPoints || m_settings->displayEllipsoids) ? m_settings->rendermodeblending : 1.0f);
	m_program_regular->setUniformValue(m_locDensityMin, m_settings->densitymin);
	m_program_regular->setUniformValue(m_locDensityMax, m_settings->densitymax);
	m_gl->glDispatchCompute(ceil(screenWidth / 32.0f), ceil(screenHeight / 32.0), 1);
	m_gl->glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	m_gl->glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo.getID());

	m_gl->glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight,
		GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

void GMDensityRaycastRenderer::enableAccelerationStructure()
{
	if (!validAccelerationStructure && m_mixture) {
		buildAccelerationStructure();
	}
	useAccelerationStructure = true;
}

void GMDensityRaycastRenderer::disableAccelerationStructure()
{
	if (useAccelerationStructure && m_mixture) {
		buildUnacceleratedData();
	}
	useAccelerationStructure = false;
}

void GMDensityRaycastRenderer::rebuildAccelerationStructure()
{
	if (m_mixture) {
		buildAccelerationStructure();
	}
	useAccelerationStructure = true;
}

void GMDensityRaycastRenderer::setAccelerationStructureEnabled(bool enabled)
{
	if (enabled) {
		enableAccelerationStructure();
	}
	else {
		disableAccelerationStructure();
	}
}

void GMDensityRaycastRenderer::cleanup()
{
	m_fbo.cleanup();
	m_program_regular.reset();
}

void GMDensityRaycastRenderer::buildAccelerationStructure()
{
	QVector<GMOctreeNode> octree;
	size_t arrsize;
	std::shared_ptr<char[]> gpudata = m_mixture->buildOctree(m_settings->accelerationthreshold, octree, arrsize);
	validAccelerationStructure = true;
	//Bind Gaussians
	m_gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboMixture);
	m_gl->glBufferData(GL_SHADER_STORAGE_BUFFER, arrsize, gpudata.get(), GL_STATIC_DRAW);
	//Bind Octree
	m_gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboOctree);
	m_gl->glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GMOctreeNode) * octree.size(), octree.constData(), GL_STATIC_DRAW);

	m_gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void GMDensityRaycastRenderer::buildUnacceleratedData()
{
	size_t arrsize;
	std::shared_ptr<char[]> gpudata = m_mixture->gpuData(arrsize);

	m_gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboMixture);
	m_gl->glBufferData(GL_SHADER_STORAGE_BUFFER, arrsize, gpudata.get(), GL_STATIC_DRAW);
	m_gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	validAccelerationStructure = false;
}



