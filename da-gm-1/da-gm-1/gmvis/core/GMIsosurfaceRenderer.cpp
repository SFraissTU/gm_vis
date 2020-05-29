#include "GMIsosurfaceRenderer.h"
#include "Helper.h"
#include "PointCloud.h"
#include <QtMath>

using namespace gmvis::core;

gmvis::core::GMIsosurfaceRenderer::GMIsosurfaceRenderer(QOpenGLFunctions_4_5_Core* gl, Camera* camera, int width, int height)
	: m_gl(gl), m_camera(camera), m_stencilFBO(gl, width, height), m_renderFBO(gl, width, height)
{
}

void gmvis::core::GMIsosurfaceRenderer::initialize()
{
	m_program_projection = std::make_unique<QOpenGLShaderProgram>();
	m_program_projection->addShaderFromSourceFile(QOpenGLShader::Vertex, "shaders/isosurface_proj.vert");
	m_program_projection->addShaderFromSourceFile(QOpenGLShader::Fragment, "shaders/isosurface_proj.frag");
	m_program_projection->link();

	m_program_sort_and_render = std::make_unique<QOpenGLShaderProgram>();
	m_program_sort_and_render->addShaderFromSourceFile(QOpenGLShader::Compute, "shaders/isosurface_sort_and_render.comp");
	m_program_sort_and_render->link();

	m_program_projection->bind();
	m_proj_locProjMatrix = m_program_projection->uniformLocation("projMatrix");
	m_proj_locViewMatrix = m_program_projection->uniformLocation("viewMatrix");
	m_proj_locImgStart = m_program_projection->uniformLocation("img_startidx");
	m_proj_locMaxFragListLen = m_program_projection->uniformLocation("maxFragmentListLength");
	/*m_proj_locInvViewMatrix = m_program_projection->uniformLocation("invViewMatrix");
	m_proj_locWidth = m_program_projection->uniformLocation("width");
	m_proj_locHeight = m_program_projection->uniformLocation("height");
	m_proj_locFov = m_program_projection->uniformLocation("fov");
	m_proj_locGaussTex = m_program_projection->uniformLocation("gaussTex");*/

	m_program_sort_and_render->bind();
	m_rend_locImgStart = m_program_sort_and_render->uniformLocation("img_startidx");
	m_rend_locImgRendered = m_program_sort_and_render->uniformLocation("img_rendered");
	m_rend_locListSize = m_program_sort_and_render->uniformLocation("listSize");
	m_rend_locWidth = m_program_sort_and_render->uniformLocation("width");
	m_rend_locHeight = m_program_sort_and_render->uniformLocation("height");
	m_rend_locIsolevel = m_program_sort_and_render->uniformLocation("isolevel");
	m_rend_locFov = m_program_sort_and_render->uniformLocation("fov");
	m_rend_locGaussTex = m_program_sort_and_render->uniformLocation("gaussTex");
	m_rend_locInvViewMatrix = m_program_sort_and_render->uniformLocation("invViewMatrix");

	//ToDo: Rest of Uniforms

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

	m_gl->glCreateBuffers(1, &m_ssboMixture);

	m_stencilFBO.initialize();
	m_stencilFBO.attachColorTexture(0);	//unused
	m_stencilFBO.attachStencilBuffer();

	m_renderFBO.initialize();
	m_renderFBO.attachColorTexture();

	m_gl->glCreateBuffers(1, &m_ssboFragmentList);
	m_gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboFragmentList);
	int* data = new int[4 * m_maxFragListLen];
	m_gl->glBufferData(GL_SHADER_STORAGE_BUFFER, 4 * 4 * m_maxFragListLen, data, GL_DYNAMIC_DRAW);
	m_gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	delete[] data;

	m_gl->glCreateBuffers(1, &m_atomListSize);
	m_gl->glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomListSize);
	GLuint value = 0;
	m_gl->glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &value, GL_DYNAMIC_DRAW);

	m_gl->glGenTextures(1, &m_imgStartIdx);
	m_gl->glBindTexture(GL_TEXTURE_2D, m_imgStartIdx);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, m_stencilFBO.getWidth(), m_stencilFBO.getHeight(), 0, GL_RED_INTEGER, GL_INT, nullptr);
	m_gl->glBindTexture(GL_TEXTURE_2D, 0);
}

void gmvis::core::GMIsosurfaceRenderer::setMixture(GaussianMixture* mixture, double accThreshold)
{
	m_mixture = mixture;
	updateAccelerationData(accThreshold);
}

void gmvis::core::GMIsosurfaceRenderer::updateAccelerationData(double accThreshold)
{
	if (!m_mixture) return;
	int n = m_mixture->numberOfGaussians();
	QVector<QMatrix4x4> transforms;
	transforms.reserve(n);
	for (int i = 0; i < n; ++i) {
		const Gaussian* gauss = (*m_mixture)[i];
		auto transform = gauss->getTransform(accThreshold);
		if (transform.has_value()) {
			transforms.push_back(transform.value());
		}
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

void gmvis::core::GMIsosurfaceRenderer::render(int screenWidth, int screenHeight)
{
	if (!m_mixture) {
		return;
	}
	if (screenWidth > m_stencilFBO.getWidth() || screenHeight > m_stencilFBO.getHeight()) {
		m_gl->glBindTexture(GL_TEXTURE_2D, m_imgStartIdx);
		m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, screenWidth, screenHeight, 0, GL_RED_INTEGER, GL_INT, nullptr);
	}
	m_stencilFBO.setSize(screenWidth, screenHeight);
	m_renderFBO.setSize(screenWidth, screenHeight);

	int value = -1;
	m_gl->glClearTexImage(m_imgStartIdx, 0, GL_RED_INTEGER, GL_INT, &value);

	//qDebug() << m_gl->glGetError() << "\n";

	GLint screenFbo = 0;
	m_gl->glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &screenFbo);
	m_gl->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_stencilFBO.getID());

	/*qDebug() << m_gl->glGetError() << "\n";

	GLint status = m_gl->glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	switch (status) {
	case GL_FRAMEBUFFER_COMPLETE:
		qDebug() << "";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		qDebug() << "A";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		qDebug() << "C";
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED:
		qDebug() << "D";
		break;
	default:
		qDebug() << "E";
	}*/
	
	m_gl->glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	m_gl->glDisable(GL_DEPTH_TEST);
	m_gl->glDisable(GL_CULL_FACE);
	m_gl->glEnable(GL_STENCIL_TEST);
	m_gl->glStencilOp(GL_INCR, GL_INCR, GL_INCR);
	m_gl->glDisable(GL_BLEND);
	m_gl->glViewport(0, 0, screenWidth, screenHeight);

	//qDebug() << m_gl->glGetError() << "\n";

	m_program_projection->bind();
	m_program_projection->setUniformValue(m_proj_locProjMatrix, m_camera->getProjMatrix());
	m_program_projection->setUniformValue(m_proj_locViewMatrix, m_camera->getViewMatrix());
	/*m_program_projection->setUniformValue(m_proj_locInvViewMatrix, m_camera->getViewMatrix().inverted());
	m_program_projection->setUniformValue(m_proj_locWidth, screenWidth);
	m_program_projection->setUniformValue(m_proj_locHeight, screenHeight);
	m_program_projection->setUniformValue(m_proj_locFov, qDegreesToRadians(m_camera->getFoV()));*/
	/*m_gl->glActiveTexture(GL_TEXTURE0);
	m_gl->glBindTexture(GL_TEXTURE_1D, m_texGauss);
	m_program_projection->setUniformValue(m_proj_locGaussTex, 0);
	m_gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboMixture);
	m_gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bindingMixture, m_ssboMixture);
	m_gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);*/

	//qDebug() << m_gl->glGetError() << "\n";

	m_gl->glUniform1ui(m_proj_locMaxFragListLen, m_maxFragListLen);
	m_gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssboFragmentList);
	m_gl->glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 1, m_atomListSize);
	m_gl->glActiveTexture(GL_TEXTURE0);
	m_gl->glBindImageTexture(0, m_imgStartIdx, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32I);
	m_gl->glUniform1i(m_proj_locImgStart, 0);

	//qDebug() << m_gl->glGetError() << "\n";

	//ToDo: We need to disable front clipping

	m_gm_vao.bind();
	m_gl->glDrawElementsInstanced(GL_TRIANGLES, m_geoIndices.count(), GL_UNSIGNED_INT, nullptr, m_nrValidMixtureComponents);

	//qDebug() << m_gl->glGetError() << "\n";

	m_gl->glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

	m_gl->glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomListSize);
	GLuint* countp = (GLuint*)m_gl->glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
	GLuint listsize = *countp;
	*countp = 0;
	m_gl->glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

	qDebug() << listsize << "\n";

	//qDebug() << m_gl->glGetError() << "\n";

	m_program_sort_and_render->bind();
	m_gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssboFragmentList);
	m_gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_ssboMixture);
	m_gl->glUniform1ui(m_rend_locListSize, listsize);
	m_gl->glUniform1ui(m_rend_locWidth, screenWidth);
	m_gl->glUniform1ui(m_rend_locHeight, screenHeight);
	m_gl->glUniform1f(m_rend_locIsolevel, m_sIsolevel);
	m_gl->glUniform1f(m_rend_locFov, qDegreesToRadians(m_camera->getFoV()));
	m_gl->glActiveTexture(GL_TEXTURE0);
	//m_gl->glBindImageTexture(0, m_stencilFBO.getColorTexture(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	m_gl->glBindImageTexture(0, m_imgStartIdx, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32I);
	m_gl->glUniform1i(m_rend_locImgStart, 0);
	//qDebug() << m_gl->glGetError() << "\n";
	m_gl->glActiveTexture(GL_TEXTURE1);
	m_gl->glBindImageTexture(1, m_renderFBO.getColorTexture(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	m_gl->glUniform1i(m_rend_locImgRendered, 1);
	m_gl->glActiveTexture(GL_TEXTURE2);
	m_gl->glBindTexture(GL_TEXTURE_1D, m_texGauss);
	m_gl->glUniform1i(m_rend_locGaussTex, 2);
	m_program_sort_and_render->setUniformValue(m_rend_locInvViewMatrix, m_camera->getViewMatrix().inverted());

	//qDebug() << m_gl->glGetError() << "\n";

	m_gl->glDispatchCompute(screenWidth / 32 + 1, screenHeight / 32 + 1, 1);
	
	//qDebug() << m_gl->glGetError() << "\n";

	//resetten von buffern!
	m_gl->glDisable(GL_STENCIL_TEST);

	m_gl->glBindFramebuffer(GL_READ_FRAMEBUFFER, m_renderFBO.getID());
	m_gl->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, screenFbo);
	m_gl->glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight,
		GL_COLOR_BUFFER_BIT, GL_LINEAR);

	//qDebug() << m_gl->glGetError() << "\n";
}

void gmvis::core::GMIsosurfaceRenderer::cleanup()
{
}

void gmvis::core::GMIsosurfaceRenderer::setIsolevel(float iso)
{
	m_sIsolevel = iso;
}

const float& gmvis::core::GMIsosurfaceRenderer::getIsolevel() const
{
	return m_sIsolevel;
}
