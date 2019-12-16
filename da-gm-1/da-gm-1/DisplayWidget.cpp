#include "DisplayWidget.h"

#include <memory>

#include "math.h"
#include "PointCloudLoader.h"

//Partialy used this: https://code.qt.io/cgit/qt/qtbase.git/tree/examples/opengl/hellogl2?h=5.13

DisplayWidget::DisplayWidget(QWidget* parent) : QOpenGLWidget(parent) {
	QSurfaceFormat format;
	format.setMajorVersion(4);
	format.setMinorVersion(6);
	format.setProfile(QSurfaceFormat::CoreProfile);
	format.setOption(QSurfaceFormat::DebugContext);
	setFormat(format);
	m_camera = std::make_unique<Camera>(60.0f, GLfloat(width()) / height(), 0.01f, 1000.0f);
}

DisplayWidget::~DisplayWidget() {
	cleanup();
}

QSize DisplayWidget::minimumSizeHint() const {
	return QSize(50, 50);
}

QSize DisplayWidget::sizeHint() const {
	return QSize(500, 500);
}

void DisplayWidget::setPointCloud(PointCloud* pointcloud)
{
	this->m_pointcloud = pointcloud;
	if (!m_initialized) {
		return;	//We'll try again later
	}
	if (!m_pc_vao.isCreated()) {
		m_pc_vao.create();
	}
	QOpenGLVertexArrayObject::Binder vaoBinder(&m_pc_vao);
	m_pc_vbo.create();
	m_pc_vbo.bind();
	m_pc_vbo.allocate(pointcloud->getData(), pointcloud->getDataSize());
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, pointcloud->getSinglePointSize(), 0);
	m_pc_vbo.release();
}

void DisplayWidget::cleanup() {
	if (m_program.get() == nullptr) return;
	makeCurrent();
	m_pc_vbo.destroy();
	m_program.reset();
	m_debugLogger.reset();
	doneCurrent();
}

void DisplayWidget::initializeGL() {
	connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &DisplayWidget::cleanup);

	initializeOpenGLFunctions();
	m_initialized = true;
	if (m_pointcloud) {
		setPointCloud(m_pointcloud);//If it has been set before initialization, try again
	}

#if _DEBUG
	m_debugLogger = std::make_unique<QOpenGLDebugLogger>(this);
	if (m_debugLogger->initialize()) {
		qDebug() << "GL_DEBUG Debug Logger " << m_debugLogger.get() << "\n";
		connect(m_debugLogger.get(), &QOpenGLDebugLogger::messageLogged, this, &DisplayWidget::messageLogged);
		m_debugLogger->startLogging();
	}
#endif

	glClearColor(0, 0, 0, 1);

	m_program = std::make_unique<QOpenGLShaderProgram>();
	m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "shaders/dummy.vert");
	m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "shaders/dummy.frag");
	//m_program->bindAttributeLocation("in_vertex", 0);
	m_program->link();

	m_program->bind();
	m_projMatrixLoc = m_program->uniformLocation("projMatrix");
	m_viewMatrixLoc = m_program->uniformLocation("viewMatrix");
	m_lightPosLoc = m_program->uniformLocation("lightPos");

	m_program->setUniformValue(m_lightPosLoc, QVector3D(0, 0, 2));
	
	m_program->release();
}

void DisplayWidget::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);

	if (!m_pc_vao.isCreated()) {
		return;
	}
	
	QOpenGLVertexArrayObject::Binder vaoBinder(&m_pc_vao);
	m_program->bind();
	m_program->setUniformValue(m_projMatrixLoc, m_camera->getProjMatrix());
	m_program->setUniformValue(m_viewMatrixLoc, m_camera->getViewMatrix());
	
	glDrawArrays(GL_POINTS, 0, m_pointcloud->getPointCount());
	m_program->release();
}

void DisplayWidget::resizeGL(int width, int height)
{
	m_camera->setAspectRatio(GLfloat(width) / height);
}

void DisplayWidget::mousePressEvent(QMouseEvent* event)
{
	m_lastPos = event->pos();
}

void DisplayWidget::mouseMoveEvent(QMouseEvent* event)
{
	int dx = event->x() - m_lastPos.x();
	int dy = event->y() - m_lastPos.y();

	bool refresh = false;
	if (event->buttons() & Qt::LeftButton) {
		m_camera->rotateX(2 * dy);
		m_camera->rotateY(2 * dx);
		refresh = true;
	}
	if (event->buttons() & Qt::MiddleButton) {
		m_camera->zoom(- 2 * dy);
		refresh = true;
	}
	if (event->buttons() & Qt::RightButton) {
		m_camera->translateAlongScreen(dx, dy);
		refresh = true;
	}
	if (refresh) {
		update();
		m_lastPos = event->pos();
	}
}

//This function is copied from http://www.trentreed.net/blog/qt5-opengl-part-5-debug-logging/
void DisplayWidget::messageLogged(const QOpenGLDebugMessage& msg) {
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
