#include "GMDensityRaycastRenderer.h"
#include "DataLoader.h"
#include <cmath>
#include <QtMath>

using namespace gmvis::core;

GMDensityRaycastRenderer::GMDensityRaycastRenderer(QOpenGLFunctions_4_5_Core* gl, Camera* camera, int width, int height) : m_gl(gl), m_camera(camera) {
	
}

void GMDensityRaycastRenderer::initialize()
{
	m_program_regular = std::make_unique<QOpenGLShaderProgram>();
	m_program_regular->addShaderFromSourceCode(QOpenGLShader::Compute, DataLoader::readRessource("shaders/density_exact.comp"));
	m_program_regular->link();

	m_program_accelerated = std::make_unique<QOpenGLShaderProgram>();
	m_program_accelerated->addShaderFromSourceCode(QOpenGLShader::Compute, DataLoader::readRessource("shaders/density_octree.comp"));
	m_program_accelerated->link();

	m_program_sampling = std::make_unique<QOpenGLShaderProgram>();
	m_program_sampling->addShaderFromSourceCode(QOpenGLShader::Compute, DataLoader::readRessource("shaders/density_sampling.comp"));
	m_program_sampling->link();

	m_program_accelerated->bind();
	//Locations should be the same in both programs
	m_locOuttex = m_program_regular->uniformLocation("img_output");
	m_bindingMixture = 0;
	m_bindingOctree = 1;
	m_locInvViewMatrix = m_program_regular->uniformLocation("invViewMatrix");
	m_locWidth = m_program_regular->uniformLocation("width");
	m_locHeight = m_program_regular->uniformLocation("height");
	m_locFov = m_program_regular->uniformLocation("fov");
	m_locGaussTex = m_program_regular->uniformLocation("gaussTex");

	m_program_regular->release();

	double factor = 1.0 / sqrt(0.5);
	float* gaussdata = new float[1001];
	gaussdata[0] = 0;
	gaussdata[1000] = 1;
	for (int i = 1; i < 1000; ++i) {
		double t = (i - 500) / 100.0;
		gaussdata[i] = 0.5 * erfc(-t / factor);
	}
	m_gl->glGenTextures(1, &m_texGauss);
	m_gl->glBindTexture(GL_TEXTURE_1D, m_texGauss);
	m_gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	m_gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	m_gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_gl->glTexImage1D(GL_TEXTURE_1D, 0, GL_RED, 1001, 0, GL_RED, GL_FLOAT, gaussdata);
	delete[] gaussdata;

	m_gl->glCreateBuffers(1, &m_ssboMixture);
	m_gl->glCreateBuffers(1, &m_ssboOctree);
}

void GMDensityRaycastRenderer::setMixture(GaussianMixture<DECIMAL_TYPE>* mixture)
{
	m_mixture = mixture;

	if (!m_useAccelerationStructure) {
		buildUnacceleratedData();
	}
	else {
		buildAccelerationStructure();
	}
}

void GMDensityRaycastRenderer::render(GLuint outTexture, int screenWidth, int screenHeight)
{
	if (!m_mixture) {
		return;
	}

	auto& currentProgram = m_useAccelerationStructure ? 
		(m_useSampling ? m_program_sampling : m_program_accelerated)
		: m_program_regular;
	
	currentProgram->bind();
	m_gl->glActiveTexture(GL_TEXTURE0);
	m_gl->glBindImageTexture(0, outTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	currentProgram->setUniformValue(m_locOuttex, 0);
	m_gl->glActiveTexture(GL_TEXTURE1);
	m_gl->glBindTexture(GL_TEXTURE_1D, m_texGauss);
	currentProgram->setUniformValue(m_locGaussTex, 1);

	m_gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboMixture);
	m_gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bindingMixture, m_ssboMixture);

	if (m_useAccelerationStructure) {
		m_gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboOctree);
		m_gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bindingOctree, m_ssboOctree);
	}

	m_gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	m_program_regular->setUniformValue(m_locWidth, screenWidth);
	m_program_regular->setUniformValue(m_locHeight, screenHeight);
	m_program_regular->setUniformValue(m_locInvViewMatrix, m_camera->getViewMatrix().inverted());
	m_program_regular->setUniformValue(m_locFov, qDegreesToRadians(m_camera->getFoV()));
	m_gl->glDispatchCompute(ceil(screenWidth / 32.0f), ceil(screenHeight / 32.0), 1);
	m_gl->glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void GMDensityRaycastRenderer::enableAccelerationStructure()
{
	if (!m_validAccelerationStructure && m_mixture) {
		buildAccelerationStructure();
	}
	m_useAccelerationStructure = true;
}

void GMDensityRaycastRenderer::disableAccelerationStructure()
{
	if (m_useAccelerationStructure && m_mixture) {
		buildUnacceleratedData();
	}
	m_useAccelerationStructure = false;
}

void GMDensityRaycastRenderer::rebuildAccelerationStructure()
{
	if (m_mixture && m_useAccelerationStructure) {
		buildAccelerationStructure();
	}
}

void GMDensityRaycastRenderer::setSampling(bool sampling)
{
	this->m_useSampling = sampling;
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

void GMDensityRaycastRenderer::setAccelerationThreshold(double threshold)
{
	m_sAccThreshold = threshold;
}

void GMDensityRaycastRenderer::cleanup()
{
	m_program_regular.reset();
	m_program_accelerated.reset();
	m_program_sampling.reset();
}

void GMDensityRaycastRenderer::buildAccelerationStructure()
{
	QVector<GMOctreeNode> octree;
	size_t arrsize;
	std::shared_ptr<char[]> gpudata = m_mixture->buildOctree(m_sAccThreshold, octree, arrsize);
	m_validAccelerationStructure = true;
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

	m_validAccelerationStructure = false;
}



