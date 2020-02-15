#include "GMDensityRendererAcc2.h"
#include "DataLoader.h"
#include "Helper.h"
#include <math.h>
#include <QtMath>

GMDensityRendererAcc2::GMDensityRendererAcc2(QOpenGLFunctions_4_5_Core* gl, DisplaySettings* settings, Camera* camera, int width, int height) : m_gl(gl), m_settings(settings), m_camera(camera), m_fbo_projection(ScreenFBO(gl, width, height)), m_fbo_final(ScreenFBO(gl, width, height)) {
	m_program_projection = std::make_unique<QOpenGLShaderProgram>();
	m_program_projection->addShaderFromSourceFile(QOpenGLShader::Vertex, "shaders/density_acs_proj.vert");
	m_program_projection->addShaderFromSourceFile(QOpenGLShader::Fragment, "shaders/density_acs_proj.frag");
	m_program_projection->link();

	m_program_coloring = std::make_unique<QOpenGLShaderProgram>();
	m_program_coloring->addShaderFromSourceFile(QOpenGLShader::Compute, "shaders/density_acs_color.comp");
	m_program_coloring->link();

	m_program_projection->bind();
	m_proj_locProjMatrix = m_program_projection->uniformLocation("projMatrix");
	m_proj_locViewMatrix = m_program_projection->uniformLocation("viewMatrix");
	m_proj_locInvViewMatrix = m_program_projection->uniformLocation("invViewMatrix");
	m_proj_locWidth = m_program_projection->uniformLocation("width");
	m_proj_locHeight = m_program_projection->uniformLocation("height");
	m_proj_locFov = m_program_projection->uniformLocation("fov");
	m_proj_locGaussTex = m_program_projection->uniformLocation("gaussTex");
	m_proj_bindingMixture = 0;

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
	m_program_coloring->release();

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

	m_fbo_projection.attachSinglevalueTexture();
	m_fbo_final.attachColorTexture();

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
	delete gaussdata;

	QVector<QVector3D> transferdata = DataLoader::readTransferFunction(QString("res/transfer.txt"));
	gl->glGenTextures(1, &m_texTransfer);
	gl->glBindTexture(GL_TEXTURE_1D, m_texTransfer);
	gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl->glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, transferdata.size(), 0, GL_RGB, GL_FLOAT, transferdata.data());

	gl->glCreateBuffers(1, &m_ssboMixture);
}

void GMDensityRendererAcc2::setMixture(GaussianMixture* mixture)
{
	m_mixture = mixture;

	int n = mixture->numberOfGaussians();
	QVector<QMatrix4x4> transforms;
	transforms.reserve(n);
	for (int i = 0; i < n; ++i) {
		const Gaussian* gauss = (*mixture)[i];
		auto transform = gauss->getTransform(m_settings->accelerationthreshold);
		if (transform.has_value()) {
			transforms.push_back(transform.value());
		}
	}

	m_transf_vbo.bind();
	m_transf_vbo.allocate(transforms.data(), transforms.size() * sizeof(QMatrix4x4));
	m_transf_vbo.release();

	size_t arrsize;
	std::shared_ptr<char[]> gpudata = m_mixture->gpuData(arrsize, m_settings->accelerationthreshold, m_nrValidMixtureComponents);

	m_gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboMixture);
	m_gl->glBufferData(GL_SHADER_STORAGE_BUFFER, arrsize, gpudata.get(), GL_DYNAMIC_DRAW);
	m_gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void GMDensityRendererAcc2::setSize(int width, int height)
{
	m_fbo_projection.setSize(width, height);
	m_fbo_final.setSize(width, height);
}

void GMDensityRendererAcc2::render(GLuint preTexture)
{
	if (!m_mixture) {
		return;
	}

	/*m_gl->glBindFramebuffer(GL_READ_FRAMEBUFFER, depthTexture);

	m_gl->glBlitFramebuffer(0, 0, m_fbo.getWidth(), m_fbo.getHeight(), 0, 0, m_fbo.getWidth(), m_fbo.getHeight(),
		GL_COLOR_BUFFER_BIT, GL_LINEAR);
	return;*/

	GLint screenFbo = 0;
	m_gl->glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &screenFbo);
	m_gl->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo_projection.getID());
	int screenWidth = m_fbo_projection.getWidth();
	int screenHeight = m_fbo_projection.getHeight();

	m_gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_gl->glDisable(GL_DEPTH_TEST);
	m_gl->glEnable(GL_CULL_FACE);
	m_gl->glCullFace(GL_BACK);
	m_gl->glEnable(GL_BLEND);
	m_gl->glBlendFunc(GL_ONE, GL_ONE);
	m_gl->glViewport(0, 0, screenWidth, screenHeight);

	m_program_projection->bind();
	m_program_projection->setUniformValue(m_proj_locProjMatrix, m_camera->getProjMatrix());
	m_program_projection->setUniformValue(m_proj_locViewMatrix, m_camera->getViewMatrix());
	m_program_projection->setUniformValue(m_proj_locInvViewMatrix, m_camera->getViewMatrix().inverted());
	m_program_projection->setUniformValue(m_proj_locWidth, screenWidth);
	m_program_projection->setUniformValue(m_proj_locHeight, screenHeight);
	m_program_projection->setUniformValue(m_proj_locFov, qDegreesToRadians(m_camera->getFoV()));
	m_gl->glActiveTexture(GL_TEXTURE0);
	m_gl->glBindTexture(GL_TEXTURE_1D, m_texGauss);
	m_program_projection->setUniformValue(m_proj_locGaussTex, 0);
	m_gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboMixture);
	m_gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_proj_bindingMixture, m_ssboMixture);
	m_gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	m_gm_vao.bind();
	m_gl->glDrawElementsInstanced(GL_TRIANGLES, m_geoIndices.count(), GL_UNSIGNED_INT, nullptr, m_nrValidMixtureComponents);

	m_program_coloring->bind();
	m_gl->glActiveTexture(GL_TEXTURE0);
	m_gl->glBindImageTexture(0, m_fbo_final.getColorTexture(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	m_program_coloring->setUniformValue(m_col_bindingOutimg, 0);
	m_gl->glActiveTexture(GL_TEXTURE1);
	m_gl->glBindImageTexture(1, m_fbo_projection.getSinglevalueTexture(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	m_program_coloring->setUniformValue(m_col_bindingSumimg, 1);
	m_gl->glActiveTexture(GL_TEXTURE2);
	m_gl->glBindImageTexture(2, preTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	m_program_coloring->setUniformValue(m_col_bindingPreimg, 2);
	m_gl->glActiveTexture(GL_TEXTURE3);
	m_gl->glBindTexture(GL_TEXTURE_1D, m_texTransfer);
	m_program_coloring->setUniformValue(m_col_locTransferTex, 3);
	
	m_program_coloring->setUniformValue(m_col_locWidth, screenWidth);
	m_program_coloring->setUniformValue(m_col_locHeight, screenHeight);
	m_program_coloring->setUniformValue(m_col_locBlend, (m_settings->displayPoints || m_settings->displayEllipsoids) ? m_settings->rendermodeblending : 1.0f);
	m_program_coloring->setUniformValue(m_col_locDensityMin, m_settings->densitymin);
	m_program_coloring->setUniformValue(m_col_locDensityMax, m_settings->densitymax);

	m_gl->glDispatchCompute(ceil(screenWidth / 32.0f), ceil(screenHeight / 32.0), 1);
	m_gl->glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	m_gl->glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo_final.getID());
	m_gl->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, screenFbo);

	m_gl->glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight,
		GL_COLOR_BUFFER_BIT, GL_LINEAR);
}


void GMDensityRendererAcc2::cleanup()
{
	m_fbo_projection.cleanup();
	m_fbo_final.cleanup();
	m_program_projection.reset();
	m_program_coloring.reset();
}
