
#include "PythonInterface.h"
#pragma push_macro("slots")
#undef slots
#include <pybind11/pybind11.h>
#pragma pop_macro("slots")

namespace py = pybind11;

#include "gmvis/core/DataLoader.h"

using namespace gmvis::pylib;
using namespace gmvis::core;

std::unique_ptr<QApplication> PythonInterface::application;
std::unique_ptr<OffscreenRenderSurface> PythonInterface::visualizer;
std::unique_ptr<gmvis::core::PointCloud> PythonInterface::pointcloud;
std::unique_ptr<gmvis::core::GaussianMixture> PythonInterface::mixture;


int main(int argc, char* argv[])
{
	PythonInterface::startVisualizer();
	PythonInterface::initialize(500, 500);
	PythonInterface::loadMixture("D:/Simon/Studium/S-11 (WS19-20)/Diplomarbeit/da-gm-1/da-gm-1/data/c_30fix.ply");
	PythonInterface::render();
	PythonInterface::exit();
}

void PythonInterface::startVisualizer() {
	int argc = 0;
	application = std::make_unique<QApplication>(argc, nullptr);
	visualizer = std::make_unique<OffscreenRenderSurface>();
}

void PythonInterface::initialize(int width, int height) {
	visualizer->initialize(width, height);
}

void PythonInterface::loadMixture(std::string path)
{
	auto newGauss = DataLoader::readGMfromPLY(QString(path.c_str()), false);
	if (newGauss) {
		mixture = std::move(newGauss);
		visualizer->getGMDensityRenderer()->setMixture(mixture.get());
		visualizer->getGMIsoellipsoidRenderer()->setMixture(mixture.get());
	}
}

void PythonInterface::render()
{
	visualizer->render();
}

void PythonInterface::exit() {
	application->exit();
}

PYBIND11_MODULE(pygmvis, m) {
	m.doc() = "pybind11 example plugin";
	m.def("start_visualizer", &PythonInterface::startVisualizer, "Starts a new visualizer instance");
	m.def("initialize", &PythonInterface::initialize, "Initializes the Visualizer", py::arg("width"), py::arg("height"));
	m.def("load_mixture", &PythonInterface::loadMixture, "Loads a Gaussian Mixture", py::arg("path"));
	m.def("render", &PythonInterface::render, "Renders an image");
	m.def("exit", &PythonInterface::exit, "Stops the visualizer");
}