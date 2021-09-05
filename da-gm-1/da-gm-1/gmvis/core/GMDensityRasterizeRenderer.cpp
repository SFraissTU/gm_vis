#include "GMDensityRasterizeRenderer.h"
#include "DataLoader.h"
#include "Helper.h"
#include <cmath>
#include <QtMath>

using namespace gmvis::core;

GMDensityRasterizeRenderer::GMDensityRasterizeRenderer(QOpenGLFunctions_4_5_Core* gl, Camera* camera, int width, int height) : m_gl(gl), m_camera(camera) {
	
}

void GMDensityRasterizeRenderer::initialize()
{
	m_program_projection = std::make_unique<QOpenGLShaderProgram>();
	m_program_projection->addShaderFromSourceCode(QOpenGLShader::Vertex, DataLoader::readRessource("shaders/density_acc_proj.vert"));
	m_program_projection->addShaderFromSourceCode(QOpenGLShader::Fragment, DataLoader::readRessource("shaders/density_acc_proj.frag"));
	m_program_projection->link();

	m_program_falloff = std::make_unique<QOpenGLShaderProgram>();
	m_program_falloff->addShaderFromSourceCode(QOpenGLShader::Vertex, DataLoader::readRessource("shaders/density_acc_proj.vert"));
	m_program_falloff->addShaderFromSourceCode(QOpenGLShader::Fragment, DataLoader::readRessource("shaders/density_acc_falloff.frag"));
	m_program_falloff->link();

	m_program_projection->bind();
	m_proj_locProjMatrix = m_program_projection->uniformLocation("projMatrix");
	m_proj_locViewMatrix = m_program_projection->uniformLocation("viewMatrix");
	m_proj_locInvViewMatrix = m_program_projection->uniformLocation("invViewMatrix");
	m_proj_locWidth = m_program_projection->uniformLocation("width");
	m_proj_locHeight = m_program_projection->uniformLocation("height");
	m_proj_locFov = m_program_projection->uniformLocation("fov");
	m_proj_locGaussTex = m_program_projection->uniformLocation("gaussTex");
	m_program_falloff->bind();
	m_proj_locKappa = m_program_falloff->uniformLocation("kappa");
	m_proj_locFar = m_program_falloff->uniformLocation("far");
	m_proj_bindingMixture = 0;

	//Create Geometry Data
	point_list normals;
	Helper::createSphere(1.0f, 32.0f, 32.0f, m_geoVertices, normals, m_geoIndices);

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

	//Create Indices VBO
	m_indices_vbo.create();
	m_indices_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	m_indices_vbo.bind();
	m_indices_vbo.allocate(m_geoIndices.data(), m_geoIndices.size() * sizeof(GLuint));

	//Create Transformations VBO (one transformation per instance)
	m_transf_vbo.create();
	m_transf_vbo.bind();
	m_transf_vbo.allocate(nullptr, 0);
	m_gl->glEnableVertexAttribArray(1);
	m_gl->glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), 0);
	m_gl->glEnableVertexAttribArray(2);
	m_gl->glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), (void*)(sizeof(float) * 4));
	m_gl->glEnableVertexAttribArray(3);
	m_gl->glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), (void*)(sizeof(float) * 8));
	m_gl->glEnableVertexAttribArray(4);
	m_gl->glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), (void*)(sizeof(float) * 12));
	m_gl->glVertexAttribDivisor(1, 1);
	m_gl->glVertexAttribDivisor(2, 1);
	m_gl->glVertexAttribDivisor(3, 1);
	m_gl->glVertexAttribDivisor(4, 1);
	m_transf_vbo.release();

	m_gm_vao.release();

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

	float* erfdata = new float[1001];
	erfdata[0] = -1;
	erfdata[1000] = 1;
	for (int i = 1; i < 1000; ++i) {
		double t = (i - 500) / 100.0;
		erfdata[i] = erf(t);
	}
	m_gl->glGenTextures(1, &m_texErf);
	m_gl->glBindTexture(GL_TEXTURE_1D, m_texErf);
	m_gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	m_gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	m_gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_gl->glTexImage1D(GL_TEXTURE_1D, 0, GL_RED, 1001, 0, GL_RED, GL_FLOAT, erfdata);
	delete[] erfdata;

	m_gl->glCreateBuffers(1, &m_ssboMixture);
}

void GMDensityRasterizeRenderer::setMixture(GaussianMixture<DECIMAL_TYPE>* mixture, double accThreshold)
{
	m_mixture = mixture;
	updateAccelerationData(accThreshold);
}

void GMDensityRasterizeRenderer::updateAccelerationData(double accThreshold)
{
	if (!m_mixture) return;
	int n = m_mixture->numberOfGaussians();
	QVector<QMatrix4x4> transforms;
	transforms.reserve(n);
	int i = m_mixture->nextEnabledGaussianIndex(-1);
	while(i != -1) {
		const Gaussian<DECIMAL_TYPE>* gauss = (*m_mixture)[i];
		auto transform = gauss->getTransform(accThreshold);
		if (transform.has_value()) {
			transforms.push_back(transform.value());
		}
		i = m_mixture->nextEnabledGaussianIndex(i);
	}

	m_transf_vbo.bind();
	m_transf_vbo.allocate(transforms.data(), transforms.size() * sizeof(QMatrix4x4));
	m_transf_vbo.release();
	
	size_t arrsize;
	std::shared_ptr<char[]> gpudata = m_mixture->gpuData(arrsize, accThreshold, m_nrValidMixtureComponents);

	m_gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboMixture);
	m_gl->glBufferData(GL_SHADER_STORAGE_BUFFER, arrsize, gpudata.get(), GL_DYNAMIC_DRAW);
	m_gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void GMDensityRasterizeRenderer::render(int screenWidth, int screenHeight)
{
	if (!m_mixture) {
		return;
	}

	m_gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_gl->glDisable(GL_DEPTH_TEST);
	m_gl->glEnable(GL_CULL_FACE);
	m_gl->glCullFace(GL_FRONT);
	m_gl->glEnable(GL_BLEND);
	m_gl->glBlendFunc(GL_ONE, GL_ONE);
	m_gl->glViewport(0, 0, screenWidth, screenHeight);

	QOpenGLShaderProgram* program = (m_useFalloff ? m_program_falloff.get() : m_program_projection.get());

	program->bind();
	program->setUniformValue(m_proj_locProjMatrix, m_camera->getProjMatrix());
	program->setUniformValue(m_proj_locViewMatrix, m_camera->getViewMatrix());
	program->setUniformValue(m_proj_locInvViewMatrix, m_camera->getViewMatrix().inverted());
	program->setUniformValue(m_proj_locWidth, screenWidth);
	program->setUniformValue(m_proj_locHeight, screenHeight);
	program->setUniformValue(m_proj_locFov, qDegreesToRadians(m_camera->getFoV()));
	m_gl->glActiveTexture(GL_TEXTURE0);
	m_gl->glBindTexture(GL_TEXTURE_1D, m_useFalloff ? m_texErf : m_texGauss);
	program->setUniformValue(m_proj_locGaussTex, 0);
	m_gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboMixture);
	m_gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_proj_bindingMixture, m_ssboMixture);
	m_gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	program->setUniformValue(m_proj_locKappa, m_kappa);
	if (m_useFalloff)
	{
		program->setUniformValue(m_proj_locFar, m_far);
	}

	m_gm_vao.bind();
	m_gl->glDrawElementsInstanced(GL_TRIANGLES, m_geoIndices.count(), GL_UNSIGNED_INT, nullptr, m_nrValidMixtureComponents);
}


void GMDensityRasterizeRenderer::cleanup()
{
	m_program_projection.reset();
}

void gmvis::core::GMDensityRasterizeRenderer::setUseFalloff(bool use)
{
	m_useFalloff = use;
}

void gmvis::core::GMDensityRasterizeRenderer::setFalloffOptions(float kappa, float farp)
{
	m_kappa = kappa;
	m_far = farp;
}
