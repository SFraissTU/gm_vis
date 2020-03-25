
#include "PythonInterface.h"
#pragma push_macro("slots")
#undef slots
#include <pybind11/pybind11.h>
#pragma pop_macro("slots")

#include <qapplication.h>

using namespace gmvis::pylib;

std::unique_ptr<OffscreenRenderSurface> PythonInterface::visualizer;


int main(int argc, char* argv[])
{
	QApplication a(argc, (char**)nullptr);

	QOffscreenSurface* surface = new QOffscreenSurface();
	surface->requestedFormat().setVersion(4, 5);
	surface->setFormat(surface->requestedFormat());

	QOpenGLContext* context = new QOpenGLContext();
	context->setFormat(surface->format());
	context->create();
	context->makeCurrent(surface);
}

void PythonInterface::startVisualizer() {
	main(0, nullptr);
	//visualizer = std::make_unique<OffscreenRenderSurface>();
}

void PythonInterface::initialize() {
	//visualizer->initialize(500, 500);
}

PYBIND11_MODULE(pygmvis, m) {
	m.doc() = "pybind11 example plugin";
	m.def("start_visualizer", &PythonInterface::startVisualizer, "Starts a new visualizer instance");
	m.def("initialize", &PythonInterface::initialize, "Initializes the Visualizer");
}