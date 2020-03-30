
#include "PythonInterface.h"
#include "gmvis/core/DataLoader.h"
#include <thread>

using namespace gmvis::pylib;
using namespace gmvis::core;
using namespace pybind11::literals;

PythonInterface PythonInterface::pinterface;


int main(int argc, char* argv[])
{
	PythonInterface::initialize(500, 500);
	PythonInterface::start_rendering();
	PythonInterface::render("D:/Simon/Studium/S-11 (WS19-20)/Diplomarbeit/da-gm-1/da-gm-1/data/c_30fix.ply");
	PythonInterface::stop_rendering();
	PythonInterface::shutdown();
}

void PythonInterface::initialize(int width, int height) {
	if (pinterface.application) return;
	int argc = 0;
	pinterface.application = std::make_unique<QApplication>(argc, nullptr);
	pinterface.visualizer = std::make_unique<OffscreenRenderSurface>();
	pinterface.visualizer->initialize(width, height);
}

void PythonInterface::set_image_size(int width, int height)
{
	if (pinterface.threadrunning) return;
	pinterface.visualizer->setSize(width, height);
}

void gmvis::pylib::PythonInterface::set_camera_auto(bool mode)
{
	//TODO
}

void gmvis::pylib::PythonInterface::set_camera_position(float x, float y, float z)
{
	//TODO
}

void gmvis::pylib::PythonInterface::set_camera_lookat(float x, float y, float z)
{
	//TODO
}

void PythonInterface::set_ellipsoid_rendering(bool ellipsoids, bool pointcloud)
{
	if (pinterface.threadrunning) return;
	pinterface.visualizer->setEllipsoidDisplayEnabled(ellipsoids);
	pinterface.visualizer->setPointDisplayEnabled(ellipsoids && pointcloud);
}

void PythonInterface::set_ellipsoid_coloring(GMIsoellipsoidRenderMode colorMode, GMIsoellipsoidColorRangeMode rangeMode, float min, float max)
{
	if (pinterface.threadrunning) return;
	pinterface.visualizer->getGMIsoellipsoidRenderer()->setRenderMode(colorMode);
	pinterface.visualizer->getGMIsoellipsoidRenderer()->setRangeMode(rangeMode);
	if (rangeMode == GMIsoellipsoidColorRangeMode::RANGE_MANUAL) {
		pinterface.visualizer->getGMIsoellipsoidRenderer()->setEllMin(min);
		pinterface.visualizer->getGMIsoellipsoidRenderer()->setEllMax(max);
	}
}

void gmvis::pylib::PythonInterface::set_density_rendering(bool density, GMDensityRenderMode renderMode)
{
	if (pinterface.threadrunning) return;
	pinterface.visualizer->setDensityDisplayEnabled(density);
	if (density) {
		pinterface.visualizer->getGMDensityRenderer()->setRenderMode(renderMode);
	}
}

void gmvis::pylib::PythonInterface::set_density_coloring(bool automatic, float autoperc, float min, float max)
{
	if (pinterface.threadrunning) return;
	pinterface.visualizer->getGMDensityRenderer()->setDensityAuto(automatic);
	if (automatic) {
		pinterface.visualizer->getGMDensityRenderer()->setDensityAutoPercentage(autoperc);
	}
	else {
		pinterface.visualizer->getGMDensityRenderer()->setDensityMin(min);
		pinterface.visualizer->getGMDensityRenderer()->setDensityMax(max);
	}
	
}

void gmvis::pylib::PythonInterface::set_density_accthreshold(bool automatic, float threshold)
{
	if (pinterface.threadrunning) return;
	pinterface.visualizer->getGMDensityRenderer()->setAccelerationThresholdAuto(automatic);
	if (automatic) {
		pinterface.visualizer->getGMDensityRenderer()->setAccelerationThreshold(threshold);
	}
}

void gmvis::pylib::PythonInterface::set_pointcloud(std::string path)
{
	if (pinterface.threadrunning) return;
	auto newPC = DataLoader::readPCDfromOFF(QString(path.c_str()), false);
	if (newPC) {
		pinterface.pointcloud = std::move(newPC);
		pinterface.visualizer->getPointCloudRenderer()->setPointCloud(pinterface.pointcloud.get());
	}
}

void gmvis::pylib::PythonInterface::set_tensorboard_writer(py::object writer)
{
	if (pinterface.threadrunning) return;
	pinterface.writer = std::make_unique<py::object>(writer);
}

void gmvis::pylib::PythonInterface::start_rendering()
{
	if (!pinterface.threadrunning && !pinterface.thread) {
		pinterface.threadrunning = true;
		pinterface.thread = std::make_unique<std::thread>(processRenderRequests);
	}
}

void gmvis::pylib::PythonInterface::render(std::string gmpath, int epoch)
{
	pinterface.renderrequests_mutex.lock();
	pinterface.renderrequests.push(std::pair<std::string, int>(gmpath, epoch));
	pinterface.renderrequests_mutex.unlock();
}

void gmvis::pylib::PythonInterface::stop_rendering()
{
	if (pinterface.threadrunning && pinterface.thread) {
		pinterface.threadrunning = false;
		pinterface.thread->join();
	}
}

void gmvis::pylib::PythonInterface::shutdown()
{
	stop_rendering();
	pinterface.application->exit();
}

void gmvis::pylib::PythonInterface::processRenderRequests()
{
	while (pinterface.threadrunning) {
		pinterface.renderrequests_mutex.lock();
		if (!pinterface.renderrequests.empty()) {
			std::pair<std::string, int> request = pinterface.renderrequests.front();
			std::string gmpath = request.first;
			int epoch = request.second;
			pinterface.renderrequests.pop();
			pinterface.renderrequests_mutex.unlock();
			auto newGauss = DataLoader::readGMfromPLY(QString(gmpath.c_str()), false);
			if (newGauss) {
				pinterface.mixture = std::move(newGauss);
				pinterface.visualizer->setMixture(pinterface.mixture.get());
				std::vector<Image> pixeldata = pinterface.visualizer->render();
				//Abspeichern!
				if (pinterface.writer) {
					py::object add_image = pinterface.writer->attr("add_image");
					if (pinterface.visualizer->isEllipsoidDisplayEnabled()) {
						add_image("a. ellipsoids", pixeldata[0], epoch, "dataformats"_a = "HWC");
					}
					if (pinterface.visualizer->isDensityDisplayEnabled()) {
						add_image("b. density", pixeldata[pixeldata.size() == 2], epoch, "dataformats"_a = "HWC");
					}
				}
			}
		}
		else {
			pinterface.renderrequests_mutex.unlock();
		}
	}
}

PYBIND11_MODULE(pygmvis, m) {
	m.doc() = "pybind11 example plugin";

	py::class_<Image>(m, "Image", py::buffer_protocol()).def_buffer(&Image::bufferInfo);

	m.def("initialize", &PythonInterface::initialize, "Initializes the Visualizer", py::arg("width") = 500, py::arg("height") = 500);
	m.def("set_mage_size", &PythonInterface::set_image_size, "width"_a, "height"_a);
	m.def("set_camera_auto", &PythonInterface::set_camera_auto, "mode"_a);
	m.def("set_camera_position", &PythonInterface::set_camera_position, "x"_a, "y"_a, "z"_a);
	m.def("set_camera_lookat", &PythonInterface::set_camera_lookat, "x"_a, "y"_a, "z"_a);
	m.def("set_ellipsoid_rendering", &PythonInterface::set_ellipsoid_rendering, "ellipsoids"_a, "pointcloud"_a = true);
	py::enum_<GMIsoellipsoidRenderMode>(m, "GMIsoellipsoidRenderMode")
		.value("COLOR_AMPLITUDE", GMIsoellipsoidRenderMode::COLOR_AMPLITUDE)
		.value("COLOR_UNIFORM", GMIsoellipsoidRenderMode::COLOR_UNIFORM)
		.value("COLOR_WEIGHT", GMIsoellipsoidRenderMode::COLOR_WEIGHT)
		.export_values();
	py::enum_<GMIsoellipsoidColorRangeMode>(m, "GMIsoellipsoidColorRangeMode")
		.value("RANGE_MANUAL", GMIsoellipsoidColorRangeMode::RANGE_MANUAL)
		.value("RANGE_MINMAX", GMIsoellipsoidColorRangeMode::RANGE_MINMAX)
		.value("RANGE_MEDMED", GMIsoellipsoidColorRangeMode::RANGE_MEDMED)
		.export_values();
	m.def("set_ellipsoid_coloring", &PythonInterface::set_ellipsoid_coloring, "colorMode"_a = GMIsoellipsoidRenderMode::COLOR_UNIFORM,
		"rangeMode"_a = GMIsoellipsoidColorRangeMode::RANGE_MANUAL, "min"_a=0.0f, "max"_a = 0.0f);

}