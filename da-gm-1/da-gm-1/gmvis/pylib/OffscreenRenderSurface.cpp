#include "OffscreenRenderSurface.h"
#include <QGuiApplication>

using namespace gmvis::pylib;
using namespace gmvis::core;

OffscreenRenderSurface::OffscreenRenderSurface() : 
	QOffscreenSurface() {
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

	m_camera = std::make_unique<Camera>(60.0f, GLfloat(width) / height, 0.01f, 1000.0f);
	m_pointcloudRenderer = std::make_unique<PointCloudRenderer>(static_cast<QOpenGLFunctions_4_5_Core*>(this), m_camera.get());
	m_isoellipsoidRenderer = std::make_unique<GMIsoellipsoidRenderer>(static_cast<QOpenGLFunctions_4_5_Core*>(this), m_camera.get());
	m_densityRenderer = std::make_unique<GMDensityRenderer>(static_cast<QOpenGLFunctions_4_5_Core*>(this), m_camera.get(), width, height);
	
	initializeOpenGLFunctions();
	
	m_pointcloudRenderer->initialize();
	m_isoellipsoidRenderer->initialize();
	m_densityRenderer->initialize();
	
	m_fbo = std::make_unique<ScreenFBO>(static_cast<QOpenGLFunctions_4_5_Core*>(this), width, height);
	m_fbo->initialize();
	m_fbo->attachColorTexture();
	m_fbo->attachDepthTexture();
	
	m_debugLogger = std::make_unique<QOpenGLDebugLogger>(this);
	if (m_debugLogger->initialize()) {
		qDebug() << "GL_DEBUG Debug Logger " << m_debugLogger.get() << "\n";
		(void)QObject::connect(m_debugLogger.get(), &QOpenGLDebugLogger::messageLogged, this, &OffscreenRenderSurface::messageLogged);
		m_debugLogger->startLogging();
	}
	
	glClearColor(0, 0, 0, 1);
}

void OffscreenRenderSurface::render() {
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo->getID());
	if (m_sDisplayPoints || m_sDisplayEllipsoids) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glDisable(GL_BLEND);
		glViewport(0, 0, m_fbo->getWidth(), m_fbo->getHeight());

		if (m_sDisplayPoints) {
			m_pointcloudRenderer->render();
		}
		if (m_sDisplayEllipsoids) {
			m_isoellipsoidRenderer->render();
		}

		//ToDo: STORE!
	}

	if (m_sDisplayDensity) {
		m_densityRenderer->render(m_fbo->getColorTexture(), false);

		//ToDo: STORE!
	}
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
	qDebug() << qPrintable(error) << "\n" << qPrintable(msg.message()) << "\n";
}