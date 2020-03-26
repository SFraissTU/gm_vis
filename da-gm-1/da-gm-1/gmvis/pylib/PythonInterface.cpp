
#include "PythonInterface.h"
#pragma push_macro("slots")
#undef slots
#include <pybind11/pybind11.h>
#pragma pop_macro("slots")

#include <QDate>

using namespace gmvis::pylib;

std::unique_ptr<QApplication> PythonInterface::application;
std::unique_ptr<OffscreenRenderSurface> PythonInterface::visualizer;


int main(int argc, char* argv[])
{
	PythonInterface::startVisualizer();
	PythonInterface::initialize();
	PythonInterface::exit();
}

void PythonInterface::startVisualizer() {
	int argc = 0;
	application = std::make_unique<QApplication>(argc, nullptr);
	visualizer = std::make_unique<OffscreenRenderSurface>();
}

void PythonInterface::initialize() {
	visualizer->initialize(500, 500);
}

void PythonInterface::exit() {
	application->exit();
}

PYBIND11_MODULE(pygmvis, m) {
	m.doc() = "pybind11 example plugin";
	m.def("start_visualizer", &PythonInterface::startVisualizer, "Starts a new visualizer instance");
	m.def("initialize", &PythonInterface::initialize, "Initializes the Visualizer");
	m.def("exit", &PythonInterface::exit, "Stops the visualizer");
}