
#include "PythonInterface.h"
#include "gmvis/core/DataLoader.h"
#include "gmvis/core/Camera.h"
#include <thread>
#include <QElapsedTimer>

using namespace gmvis::pylib;
using namespace gmvis::core;
using namespace pybind11::literals;

PythonInterface PythonInterface::pinterface;


int main(int argc, char* argv[])
{
	PythonInterface::initialize(true, 500, 500);
	PythonInterface::render("D:/Simon/Studium/S-11 (WS19-20)/Diplomarbeit/da-gm-1/da-gm-1/data/c_30fix.ply", 1);
	PythonInterface::finish();
}

void PythonInterface::initialize(bool async, int width, int height) {
	if (pinterface.application) return;
	pinterface.async = async;
	if (async) {
		pinterface.threadrunning = true;
		pinterface.thread = std::make_unique<std::thread>(visThread);
	}
	//Default values
	pushCommand([width, height] {
		//Start Application
		int argc = 0;
		pinterface.application = std::make_unique<QApplication>(argc, nullptr);
		pinterface.visualizer = std::make_unique<OffscreenRenderSurface>();
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
	pushCommand([mode]() {
		pinterface.cameraAuto = mode;
		if (pinterface.pointcloud) {
			calculateCameraPositionByPointcloud();
		}
	});
}

void gmvis::pylib::PythonInterface::set_camera_position(float lookat_x, float lookat_y, float lookat_z, float xRot, float yRot, float radius)
{
	pushCommand([=]() {
		auto camera = pinterface.visualizer->getCamera();
		camera->setTranslation(QVector3D(lookat_x, lookat_y, lookat_z));
		camera->setXRotation(xRot);
		camera->setYRotation(yRot);
		camera->setRadius(radius);
	});
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
			if (pinterface.cameraAuto) {
				calculateCameraPositionByPointcloud();
			}
		}
	});
}

void gmvis::pylib::PythonInterface::set_callback(py::object callback)
{
	pushCommand([callback] {
		pinterface.callback = std::make_unique<py::object>(callback);
	});
}

void gmvis::pylib::PythonInterface::render(std::string gmpath, int epoch)
{
	pushCommand([gmpath, epoch] {
		pinterface.processRenderRequest(gmpath, epoch);
	});
}

void gmvis::pylib::PythonInterface::finish()
{
	pushCommand([] {
		cleanup();
	});
	if (pinterface.thread) {
		pinterface.thread->join();
		pinterface.thread.reset();
	}
}

void gmvis::pylib::PythonInterface::forceStop()
{
	//delete all other commands
	pinterface.commandrequests_mutex.lock();
	pinterface.commandrequests = std::queue<std::function<void()>>();
	pinterface.commandrequests_mutex.unlock();
	pushCommand([] {
		cleanup();
	});
	if (pinterface.thread) {
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
	if (pinterface.async) {
		pinterface.commandrequests_mutex.lock();
		pinterface.commandrequests.push(cmd);
		pinterface.commandrequests_mutex.unlock();
	}
	else {
		cmd();
	}
}

void gmvis::pylib::PythonInterface::processRenderRequest(std::string gmpath, int epoch)
{
	auto newGauss = DataLoader::readGMfromPLY(QString(gmpath.c_str()), false);
	if (newGauss) {
		pinterface.mixture = std::move(newGauss);
		pinterface.visualizer->setMixture(pinterface.mixture.get());
		QElapsedTimer timer;
		timer.start();
		std::vector<std::unique_ptr<Image>> pixeldata = pinterface.visualizer->render();
		auto ms = timer.elapsed();
		pyprint("Rendering time: " + std::to_string(ms) + "ms");
		//Save!
		if (pinterface.callback) {
			if (pinterface.visualizer->isEllipsoidDisplayEnabled()) {
				(*pinterface.callback)(epoch, pixeldata[0]->toNpArray(), 0);
			}
			if (pinterface.visualizer->isDensityDisplayEnabled()) {
				(*pinterface.callback)(epoch, pixeldata[pixeldata.size() == 2]->toNpArray(), 1);
			}
		}
		pyprint("Visualizer: GM rendered");
	}
	else {
		pyprint("Visualizer: GM could not be read");
	}
}

void gmvis::pylib::PythonInterface::visThread()
{
	pyprint("Visualizer: Thread started!");

	//As long as thread is still running
	while (pinterface.threadrunning) {

		//Process current commands, until none are left
		pinterface.commandrequests_mutex.lock();
		if (!pinterface.commandrequests.empty()) {
			std::function<void()> func = pinterface.commandrequests.front();
			pinterface.commandrequests.pop();
			pinterface.commandrequests_mutex.unlock();
			func();
		}
		else {
			pinterface.commandrequests_mutex.unlock();
		}
	}
	pyprint("Visualizer: Thread stopped");
}

void gmvis::pylib::PythonInterface::calculateCameraPositionByPointcloud()
{
	if (pinterface.pointcloud) {
		QVector3D min = pinterface.pointcloud->getBBMin();
		QVector3D max = pinterface.pointcloud->getBBMax();
		QVector3D center = (min + max) / 2.0;
		QVector3D extend = (min - max) / 2.0;
		auto cam = pinterface.visualizer->getCamera();
		cam->setTranslation(center);
		cam->setRadius(std::max(extend.x(), std::max(extend.y(), extend.z())) * 5);
		cam->setXRotation(45);
		cam->setYRotation(-135);
	}
}

void gmvis::pylib::PythonInterface::cleanup()
{
	pinterface.application->exit();
	pinterface.mixture.reset();
	pinterface.pointcloud.reset();
	pinterface.visualizer.reset();
	pinterface.application.reset();
	pinterface.callback.reset();
	pinterface.threadrunning = false;
}

PYBIND11_MODULE(pygmvis, m) {
	m.doc() = "GM Visualizer";

	m.def("initialize", &PythonInterface::initialize, "Initializes the Visualizer", "async"_a, py::arg("width") = 500, py::arg("height") = 500);
	m.def("set_mage_size", &PythonInterface::set_image_size, "width"_a, "height"_a);
	m.def("set_camera_auto", &PythonInterface::set_camera_auto, "mode"_a);
	m.def("set_camera_position", &PythonInterface::set_camera_position, "lookat_x"_a, "lookat_y"_a, "lookat_z"_a, "xRot"_a, "yRot"_a, "radius"_a);
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
	m.def("set_callback", &PythonInterface::set_callback, "callback"_a);
	m.def("render", &PythonInterface::render, "gmpath"_a, "epoch"_a);
	m.def("finish", &PythonInterface::finish);
	m.def("forceStop", &PythonInterface::forceStop);
	m.def("buffertest", &PythonInterface::buffertest);
}