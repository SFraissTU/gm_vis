#include "OffscreenRenderSurface.h"
#include <QGuiApplication>
#include "lodepng.h"

using namespace gmvis::pylib;
using namespace gmvis::core;

OffscreenRenderSurface::OffscreenRenderSurface() : 
	QOffscreenSurface() {
    if (!isValid())
        qFatal("QOffscreenSurface is invalid!");
	QSurfaceFormat format;
	format.setMajorVersion(4);
	format.setMinorVersion(5);
	format.setProfile(QSurfaceFormat::CoreProfile);
	format.setOption(QSurfaceFormat::DebugContext);
	setFormat(format);
}

OffscreenRenderSurface::~OffscreenRenderSurface() {
	cleanup();
}

void OffscreenRenderSurface::initialize(int width, int height) {
	create();
	m_context = std::make_unique<QOpenGLContext>();
	m_context->setFormat(format());
	m_context->create();
	m_context->makeCurrent(static_cast<QOffscreenSurface*>(this));
	setOwningContext(m_context.get());

	m_camera = std::make_unique<Camera>(60.0f, GLfloat(width) / height, 0.01f, 100000.0f);
	m_pointcloudRenderer = std::make_unique<PointCloudRenderer>(static_cast<QOpenGLFunctions_4_5_Core*>(this), m_camera.get());
	m_isoellipsoidRenderer = std::make_unique<GMIsoellipsoidRenderer>(static_cast<QOpenGLFunctions_4_5_Core*>(this), m_camera.get());
	m_positionRenderer = std::make_unique<GMPositionsRenderer>(static_cast<QOpenGLFunctions_4_5_Core*>(this), m_camera.get());
	m_densityRenderer = std::make_unique<GMDensityRenderer>(static_cast<QOpenGLFunctions_4_5_Core*>(this), m_camera.get(), width, height);
	
	initializeOpenGLFunctions();
	
	m_pointcloudRenderer->initialize();
	m_isoellipsoidRenderer->initialize();
	m_positionRenderer->initialize();
	m_densityRenderer->initialize();
	
	m_fbo = std::make_unique<ScreenFBO>(static_cast<QOpenGLFunctions_4_5_Core*>(this), width, height);
	m_fbo->initialize();
	m_fbo->attachColorTexture();
	m_fbo->attachDepthTexture();
	
#ifdef _DEBUG
	m_debugLogger = std::make_unique<QOpenGLDebugLogger>(this);
	if (m_debugLogger->initialize()) {
		//qDebug() << "GL_DEBUG Debug Logger " << m_debugLogger.get() << "\n";
		(void)QObject::connect(m_debugLogger.get(), &QOpenGLDebugLogger::messageLogged, this, &OffscreenRenderSurface::messageLogged);
		m_debugLogger->startLogging();
	}
#endif
	
	glClearColor(0, 0, 0, 1);
}

Camera* gmvis::pylib::OffscreenRenderSurface::getCamera()
{
	return m_camera.get();
}

void gmvis::pylib::OffscreenRenderSurface::setSize(int width, int height)
{
	m_camera->setAspectRatio(GLfloat(width) / height);
	m_fbo->setSize(width, height);
	m_densityRenderer->setSize(width, height);
}

void gmvis::pylib::OffscreenRenderSurface::setMixture(core::GaussianMixture<DECIMAL_TYPE>* mixture)
{
	m_isoellipsoidRenderer->setMixture(mixture);
	m_positionRenderer->setMixture(mixture);
	m_densityRenderer->setMixture(mixture);
}

void gmvis::pylib::OffscreenRenderSurface::setPointcloud(core::PointCloud* pointcloud)
{
	m_pointcloudRenderer->setPointCloud(pointcloud);
}

std::vector<std::unique_ptr<Image>> OffscreenRenderSurface::render() {
	std::vector<std::unique_ptr<Image>> images;
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo->getID());
	int width = m_fbo->getWidth();
	int height = m_fbo->getHeight();
	int index = 0;
	if (m_sDisplayEllipsoids) {
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glDisable(GL_BLEND);
		glViewport(0, 0, width, height); 
		glClearColor(0.65, 0.65, 0.65, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_isoellipsoidRenderer->render();
		if (m_sDisplayEllipsoids_Points) {
			m_pointcloudRenderer->render(true);
		}
		images.push_back(std::make_unique<Image>(width, height, 4));
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo->getID());
		glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, images[index]->data());
		images[index]->clamp({ 0.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
		images[index]->invertHeight();
		index++;
	}

	if (m_sDisplayGMPositions) {
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glViewport(0, 0, width, height);
		glClearColor(0.65, 0.65, 0.65, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (m_sDisplayGMPositions_Points) {
			m_pointcloudRenderer->render(false);
		}
		m_positionRenderer->render();

		images.push_back(std::make_unique<Image>(width, height, 4));
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo->getID());
		glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, images[index]->data());
		images[index]->clamp({ 0.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
		images[index]->invertHeight();
		index++;
	}

	if (m_sDisplayDensity) {
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_densityRenderer->render(m_fbo->getColorTexture(), false);

		if (m_densityRenderer->getDensityAuto() && m_densityRenderer->getAccelerationThresholdAuto()) {
			m_densityRenderer->render(m_fbo->getColorTexture(), false);
		}

		images.push_back(std::make_unique<Image>(width, height, 4));
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo->getID());
		glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, images[index]->data());
		images[index]->clamp({ 0.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
		images[index]->invertHeight();
		index++;
	}
	return std::move(images);
}

int gmvis::pylib::OffscreenRenderSurface::getWidth() const
{
	return m_fbo->getWidth();
}

int gmvis::pylib::OffscreenRenderSurface::getHeight() const
{
	return m_fbo->getHeight();
}

PointCloudRenderer* OffscreenRenderSurface::getPointCloudRenderer()
{
	return m_pointcloudRenderer.get();
}

GMIsoellipsoidRenderer* OffscreenRenderSurface::getGMIsoellipsoidRenderer()
{
	return m_isoellipsoidRenderer.get();
}

GMPositionsRenderer* OffscreenRenderSurface::getGMPositionsRenderer()
{
	return m_positionRenderer.get();
}

GMDensityRenderer* OffscreenRenderSurface::getGMDensityRenderer()
{
	return m_densityRenderer.get();
}

void OffscreenRenderSurface::setEllipsoidDisplayEnabled(bool enabled, bool displayPoints)
{
	m_sDisplayEllipsoids = enabled;
	m_sDisplayEllipsoids_Points = displayPoints;
}

void gmvis::pylib::OffscreenRenderSurface::setGMPositionsDisplayEnabled(bool enabled, bool displayPoints)
{
	m_sDisplayGMPositions = enabled;
	m_sDisplayGMPositions_Points = displayPoints;
}

void OffscreenRenderSurface::setDensityDisplayEnabled(bool enabled)
{
	m_sDisplayDensity = enabled;
}

bool gmvis::pylib::OffscreenRenderSurface::isEllipsoidDisplayEnabled() const
{
	return m_sDisplayEllipsoids;
}

bool gmvis::pylib::OffscreenRenderSurface::isGMPositionsDisplayEnabled() const
{
	return m_sDisplayGMPositions;
}

bool gmvis::pylib::OffscreenRenderSurface::isDensityDisplayEnabled() const
{
	return m_sDisplayDensity;
}

void OffscreenRenderSurface::storeImage(std::string filename)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo->getID());
	int width = m_fbo->getWidth();
	int height = m_fbo->getHeight();
	float* pixels = new float[width * height * 4];
	glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, pixels);
	std::vector<unsigned char> image;
	image.resize(width * height * 4);
	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < height; ++y) {
			int idx = (4 * width * y) + (4 * x);
			int glidx = (4 * width * (height - 1 -y)) + 4 * x;
			image[idx + 0] = std::min(int(pixels[glidx + 0]*255.0f), 255);
			image[idx + 1] = std::min(int(pixels[glidx + 1]*255.0f), 255);
			image[idx + 2] = std::min(int(pixels[glidx + 2]*255.0f), 255);
			image[idx + 3] = 255;
		}
	}
	lodepng::encode(filename, image, width, height);
	delete pixels;
}

void OffscreenRenderSurface::cleanup() {
	m_context->makeCurrent(this);
	m_pointcloudRenderer->cleanup();
	m_isoellipsoidRenderer->cleanup();
	m_densityRenderer->cleanup();
	m_fbo->cleanup();
	m_context->doneCurrent();
}


void OffscreenRenderSurface::messageLogged(const QOpenGLDebugMessage& msg) {
	if (msg.id() == 131169 || msg.id() == 131185 || msg.id() == 131218 || msg.id() == 131204) {
		return;
	}

	QString error;
	
	// Format based on severity
	switch (msg.severity())
	{
	case QOpenGLDebugMessage::NotificationSeverity:
		error += "--";
		break;
	case QOpenGLDebugMessage::HighSeverity:
		error += "!!";
		break;
	case QOpenGLDebugMessage::MediumSeverity:
		error += "!~";
		break;
	case QOpenGLDebugMessage::LowSeverity:
		error += "~~";
		break;
	}

	error += " (";

	// Format based on source
#define CASE(c) case QOpenGLDebugMessage::c: error += #c; break
	switch (msg.source())
	{
		CASE(APISource);
		CASE(WindowSystemSource);
		CASE(ShaderCompilerSource);
		CASE(ThirdPartySource);
		CASE(ApplicationSource);
		CASE(OtherSource);
		CASE(InvalidSource);
	}
#undef CASE

	error += " : ";

	// Format based on type
#define CASE(c) case QOpenGLDebugMessage::c: error += #c; break
	switch (msg.type())
	{
		CASE(ErrorType);
		CASE(DeprecatedBehaviorType);
		CASE(UndefinedBehaviorType);
		CASE(PortabilityType);
		CASE(PerformanceType);
		CASE(OtherType);
		CASE(MarkerType);
		CASE(GroupPushType);
		CASE(GroupPopType);
	}
#undef CASE

	error += ")";
	QTextStream str;
	str << qPrintable(error) << "\n" << qPrintable(msg.message()) << "\n";
	pyprint(str.string()->toStdString());
}
