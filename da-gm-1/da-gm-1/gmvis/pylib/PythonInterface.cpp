
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
	PythonInterface::render("D:/Simon/Studium/S-11 (WS19-20)/Diplomarbeit/da-gm-1/da-gm-1/data/c_30fix.ply", 1);
	PythonInterface::finish();
}

void PythonInterface::initialize(int width, int height) {
	if (pinterface.application) return;
	pinterface.threadrunning = true;
	pinterface.thread = std::make_unique<std::thread>(visThread);
	//Default values
	pushCommand([width, height] {
		pinterface.visualizer->initialize(width, height);
	});
	set_ellipsoid_rendering(true, true);
	set_ellipsoid_coloring(GMIsoellipsoidRenderMode::COLOR_UNIFORM, GMIsoellipsoidColorRangeMode::RANGE_MANUAL, 0.0f, 1.0f);
	set_density_rendering(true, GMDensityRenderMode::ADDITIVE_EXACT);
	set_density_coloring(true, 0.9f, 0.0f, 0.5f);
}

void PythonInterface::set_image_size(int width, int height)
{
	pushCommand([width, height]() {
		pinterface.visualizer->setSize(width, height);
	});
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
	pushCommand([ellipsoids, pointcloud] {
		pinterface.visualizer->setEllipsoidDisplayEnabled(ellipsoids);
		pinterface.visualizer->setPointDisplayEnabled(ellipsoids && pointcloud);
	});
}

void PythonInterface::set_ellipsoid_coloring(GMIsoellipsoidRenderMode colorMode, GMIsoellipsoidColorRangeMode rangeMode, float min, float max)
{
	pushCommand([colorMode, rangeMode, min, max] {
		pinterface.visualizer->getGMIsoellipsoidRenderer()->setRenderMode(colorMode);
		pinterface.visualizer->getGMIsoellipsoidRenderer()->setRangeMode(rangeMode);
		if (rangeMode == GMIsoellipsoidColorRangeMode::RANGE_MANUAL) {
			pinterface.visualizer->getGMIsoellipsoidRenderer()->setEllMin(min);
			pinterface.visualizer->getGMIsoellipsoidRenderer()->setEllMax(max);
		}
	});
}

void gmvis::pylib::PythonInterface::set_density_rendering(bool density, GMDensityRenderMode renderMode)
{
	pushCommand([density, renderMode] {
		pinterface.visualizer->setDensityDisplayEnabled(density);
		if (density) {
			pinterface.visualizer->getGMDensityRenderer()->setRenderMode(renderMode);
		}
	});
	
}

void gmvis::pylib::PythonInterface::set_density_coloring(bool automatic, float autoperc, float min, float max)
{
	pushCommand([automatic, autoperc, min, max] {
		pinterface.visualizer->getGMDensityRenderer()->setDensityAuto(automatic);
		if (automatic) {
			pinterface.visualizer->getGMDensityRenderer()->setDensityAutoPercentage(autoperc);
		}
		else {
			pinterface.visualizer->getGMDensityRenderer()->setDensityMin(min);
			pinterface.visualizer->getGMDensityRenderer()->setDensityMax(max);
		}
	});
	
	
}

void gmvis::pylib::PythonInterface::set_density_accthreshold(bool automatic, float threshold)
{
	pushCommand([automatic, threshold] {
		pinterface.visualizer->getGMDensityRenderer()->setAccelerationThresholdAuto(automatic);
		if (automatic) {
			pinterface.visualizer->getGMDensityRenderer()->setAccelerationThreshold(threshold);
		}
	});	
}

void gmvis::pylib::PythonInterface::set_pointcloud(std::string path)
{
	pushCommand([path] {
		auto newPC = DataLoader::readPCDfromOFF(QString(path.c_str()), false);
		if (newPC) {
			pinterface.pointcloud = std::move(newPC);
			pinterface.visualizer->getPointCloudRenderer()->setPointCloud(pinterface.pointcloud.get());
		}
	});
}

void gmvis::pylib::PythonInterface::set_tensorboard_writer(py::object writer)
{
	pushCommand([writer] {
		pinterface.writer = std::make_unique<py::object>(writer);
	});
}

void gmvis::pylib::PythonInterface::render(std::string gmpath, int epoch)
{
	pinterface.renderrequests_mutex.lock();
	pinterface.renderrequests.push(std::pair<std::string, int>(gmpath, epoch));
	pinterface.renderrequests_mutex.unlock();
}

void gmvis::pylib::PythonInterface::finish()
{
	if (pinterface.threadrunning && pinterface.thread) {
		bool waiting = true;
		while (waiting) {
			pinterface.renderrequests_mutex.lock();
			waiting = !pinterface.renderrequests.empty();
			pinterface.renderrequests_mutex.unlock();
		}
		waiting = true;
		while (waiting) {
			pinterface.commandrequests_mutex.lock();
			waiting = !pinterface.commandrequests.empty();
			pinterface.commandrequests_mutex.unlock();
		}
		pinterface.threadrunning = false;
		pinterface.thread->join();
		pinterface.thread.reset();
	}
}

void gmvis::pylib::PythonInterface::shutdown()
{
	if (pinterface.threadrunning && pinterface.thread) {
		pinterface.threadrunning = false;
		pinterface.thread->join();
		pinterface.thread.reset();
	}
}

py::array_t<float> gmvis::pylib::PythonInterface::buffertest()
{
	py::print("This is a test");
	Image image = Image(2, 3, 4);
	float* data = image.data();
	for (int i = 0; i < 2 * 3 * 4; i++) {
		data[i] = i;
	}
	return image.toNpArray();
}

void gmvis::pylib::PythonInterface::pushCommand(std::function<void()> cmd)
{
	pinterface.commandrequests_mutex.lock();
	pinterface.commandrequests.push(cmd);
	pinterface.commandrequests_mutex.unlock();
}

void gmvis::pylib::PythonInterface::visThread()
{
	pyprint("Visualizer: Thread started!");
	//Start Application
	int argc = 0;
	pinterface.application = std::make_unique<QApplication>(argc, nullptr);
	pinterface.visualizer = std::make_unique<OffscreenRenderSurface>();

	//As long as thread is still running
	while (pinterface.threadrunning) {

		//Process current commands, until none are left
		pinterface.commandrequests_mutex.lock();
		while (pinterface.threadrunning && !pinterface.commandrequests.empty()) {
			if (!pinterface.commandrequests.empty()) {
				std::function<void()> func = pinterface.commandrequests.front();
				pinterface.commandrequests.pop();
				pinterface.commandrequests_mutex.unlock();
				func();
			}
			else {
				pinterface.commandrequests_mutex.unlock();
			}
			pinterface.commandrequests_mutex.lock();
		}
		pinterface.commandrequests_mutex.unlock();

		//Process one render request
		pinterface.renderrequests_mutex.lock();
		if (!pinterface.renderrequests.empty()) {
			std::pair<std::string, int> request = pinterface.renderrequests.front();
			std::string gmpath = request.first;
			int epoch = request.second;
			pyprint("Visualizer: Processing Request for Epoch " + std::to_string(epoch));
			pinterface.renderrequests.pop();
			pinterface.renderrequests_mutex.unlock();
			auto newGauss = DataLoader::readGMfromPLY(QString(gmpath.c_str()), false);
			if (newGauss) {
				pinterface.mixture = std::move(newGauss);
				pinterface.visualizer->setMixture(pinterface.mixture.get());
				std::vector<std::unique_ptr<Image>> pixeldata = pinterface.visualizer->render();
				//Save!
				if (pinterface.writer) {
					py::object add_image = pinterface.writer->attr("add_image");
					if (pinterface.visualizer->isEllipsoidDisplayEnabled()) {
						add_image("a. ellipsoids", pixeldata[0]->toNpArray(), epoch, "dataformats"_a = "HWC");
					}
					if (pinterface.visualizer->isDensityDisplayEnabled()) {
						add_image("b. density", pixeldata[pixeldata.size() == 2]->toNpArray(), epoch, "dataformats"_a = "HWC");
					}
					pinterface.writer->attr("flush")();
				}
				pyprint("Visualizer: GM rendered");
			}
			else {
				pyprint("Visualizer: GM could not be read");
			}
		}
		else {
			pinterface.renderrequests_mutex.unlock();
		}
	}
	pinterface.application->exit();
	pinterface.mixture.reset();
	pinterface.pointcloud.reset();
	pinterface.visualizer.reset();
	pinterface.application.reset();
	pyprint("Visualizer: Thread stopped");
}

PYBIND11_MODULE(pygmvis, m) {
	m.doc() = "GM Visualizer";

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
	py::enum_<GMDensityRenderMode>(m, "GMDensityRenderMode")
		.value("ADDITIVE_EXACT", GMDensityRenderMode::ADDITIVE_EXACT)
		.value("ADDITIVE_ACC_OCTREE", GMDensityRenderMode::ADDITIVE_ACC_OCTREE)
		.value("ADDITIVE_ACC_PROJECTED", GMDensityRenderMode::ADDITIVE_ACC_PROJECTED);
	m.def("set_density_rendering", &PythonInterface::set_density_rendering, "density"_a = true, "renderMode"_a = GMDensityRenderMode::ADDITIVE_EXACT);
	m.def("set_density_coloring", &PythonInterface::set_density_coloring, "automatic"_a = true, "autoperc"_a = 0.9, "min"_a = 0.0, "max"_a = 0.5);
	m.def("set_density_accthreshold", &PythonInterface::set_density_accthreshold, "automatic"_a = true, "threshold"_a = 0.00001);
	m.def("set_pointcloud", &PythonInterface::set_pointcloud, "path"_a);
	m.def("set_tensorboard_writer", &PythonInterface::set_tensorboard_writer, "writer"_a);
	m.def("render", &PythonInterface::render, "gmpath"_a, "epoch"_a);
	m.def("finish", &PythonInterface::finish);
	m.def("shutdown", &PythonInterface::shutdown);
	m.def("buffertest", &PythonInterface::buffertest);
}