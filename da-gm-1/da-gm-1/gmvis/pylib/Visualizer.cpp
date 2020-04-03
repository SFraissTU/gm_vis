#include "Visualizer.h"
#include <QElapsedTimer>
#include <strstream>

using namespace gmvis::pylib;
using namespace gmvis::core;

Visualizer gmvis::pylib::Visualizer::create(bool async, int width, int height)
{
	return Visualizer(async, width, height);
}

Visualizer::Visualizer(bool async, int width, int height)
{
	m_async = async;
	if (async) {
		m_threadrunning = true;
		m_thread = std::make_unique<std::thread>([this] {
			visThread();
		});
	}
	//Default values
	pushCommand([this, width, height] {
		//Start Application
		int argc = 0;
		m_application = std::make_unique<QApplication>(argc, nullptr);
		m_surface = std::make_unique<OffscreenRenderSurface>();
		m_surface->initialize(width, height);
	});
	set_ellipsoid_rendering(true, true);
	set_ellipsoid_coloring(GMIsoellipsoidRenderMode::COLOR_UNIFORM, GMIsoellipsoidColorRangeMode::RANGE_MANUAL, 0.0f, 1.0f);
	set_density_rendering(true, GMDensityRenderMode::ADDITIVE_EXACT);
	set_density_coloring(true, 0.9f, 0.0f, 0.5f);
}

void Visualizer::set_image_size(int width, int height)
{
	pushCommand([this, width, height]() {
		m_surface->setSize(width, height);
		});
}

void Visualizer::set_view_matrix(py::array_t<float> viewmat)
{
	pushCommand([this, viewmat]() {
		auto mat = viewmat.unchecked<2>();
		float values[16] = {
			mat(0, 0), mat(0, 1), mat(0, 2), mat(0, 3),
			mat(1, 0), mat(1, 1), mat(1, 2), mat(1, 3),
			mat(2, 0), mat(2, 1), mat(2, 2), mat(2, 3),
			mat(3, 0), mat(3, 1), mat(3, 2), mat(3, 3)
		};
		QMatrix4x4 qmat = QMatrix4x4(values);
		m_surface->getCamera()->setViewMatrix(qmat);
		m_cameraAuto = false;
	});
}

void Visualizer::set_camera_auto(bool camauto)
{
	pushCommand([this, camauto]() {
		m_cameraAuto = camauto;
	});
}

void Visualizer::set_ellipsoid_rendering(bool ellipsoids, bool pointcloud)
{
	pushCommand([this, ellipsoids, pointcloud] {
		m_surface->setPointDisplayEnabled(ellipsoids && pointcloud);
		m_surface->setEllipsoidDisplayEnabled(ellipsoids);
	});
}

void Visualizer::set_ellipsoid_coloring(GMIsoellipsoidRenderMode colorMode, GMIsoellipsoidColorRangeMode rangeMode, float min, float max)
{
	pushCommand([this, colorMode, rangeMode, min, max] {
		m_surface->getGMIsoellipsoidRenderer()->setRenderMode(colorMode);
		m_surface->getGMIsoellipsoidRenderer()->setRangeMode(rangeMode);
		if (rangeMode == GMIsoellipsoidColorRangeMode::RANGE_MANUAL) {
			m_surface->getGMIsoellipsoidRenderer()->setEllMin(min);
			m_surface->getGMIsoellipsoidRenderer()->setEllMax(max);
		}
	});
}

void Visualizer::set_density_rendering(bool density, GMDensityRenderMode renderMode)
{
	pushCommand([this, density, renderMode] {
		m_surface->setDensityDisplayEnabled(density);
		if (density) {
			m_surface->getGMDensityRenderer()->setRenderMode(renderMode);
		}
	});

}

void Visualizer::set_density_coloring(bool automatic, float autoperc, float min, float max)
{
	pushCommand([this, automatic, autoperc, min, max] {
		m_surface->getGMDensityRenderer()->setDensityAuto(automatic);
		if (automatic) {
			m_surface->getGMDensityRenderer()->setDensityAutoPercentage(autoperc);
		}
		else {
			m_surface->getGMDensityRenderer()->setDensityMin(min);
			m_surface->getGMDensityRenderer()->setDensityMax(max);
		}
		});


}

void Visualizer::set_density_accthreshold(bool automatic, float threshold)
{
	pushCommand([this, automatic, threshold] {
		m_surface->getGMDensityRenderer()->setAccelerationThresholdAuto(automatic);
		if (automatic) {
			m_surface->getGMDensityRenderer()->setAccelerationThreshold(threshold);
		}
		});
}

void Visualizer::set_pointclouds(py::array_t<float> pointclouds)
{
	pushCommand([this, pointclouds] {
		auto pc = pointclouds.unchecked<3>();
		auto shape = pointclouds.shape();
		size_t batchsize = shape[0];
		size_t pointcount = shape[1];
		m_pointclouds.clear();
		for (int i = 0; i < batchsize; ++i) {
			point_list pointlist = point_list();
			for (int j = 0; j < pointcount; ++j) {
				pointlist.push_back(point_item(pc(i, j, 0), pc(i, j, 1), pc(i, j, 2)));
			}
			m_pointclouds.push_back(std::make_unique<PointCloud>(pointlist));
		}
	});
}

void Visualizer::set_pointclouds_from_paths(py::list paths)
{
	pushCommand([this, paths] {
		m_pointclouds.clear();
		for (int i = 0; i < paths.size(); ++i) {
			auto newPC = DataLoader::readPCDfromOFF(QString(paths[i].cast<std::string>().c_str()), false);
			if (newPC) {
				m_pointclouds.push_back(std::move(newPC));
			}
		}
	});
}

void Visualizer::set_gaussian_mixtures(py::array_t<float> mixtures)
{
	//THIS EXPECTS WEIGHTS TO BE GMM-WEIGHTS, NOT AMPLITUDES!
	pushCommand([this, mixtures] {
		auto gms = mixtures.unchecked<3>();
		auto shape = mixtures.shape();
		size_t batchsize = shape[0];
		size_t gausscount = shape[1];
		m_mixtures.clear();
		for (int i = 0; i < batchsize; ++i) {
			GaussianMixture* mixture = new GaussianMixture();
			for (int j = 0; j < gausscount; ++j) {
				Gaussian gauss;
				gauss.weight = gms(i, j, 0);
				gauss.mux = gms(i, j, 1);
				gauss.muy = gms(i, j, 2);
				gauss.muz = gms(i, j, 3);
				gauss.covxx = gms(i, j, 4);
				gauss.covxy = gms(i, j, 5);
				gauss.covxz = gms(i, j, 6);
				gauss.covyy = gms(i, j, 8);
				gauss.covyz = gms(i, j, 9);
				gauss.covzz = gms(i, j, 12);
				gauss.finalizeInitialization();
				mixture->addGaussian(gauss);
			}
			m_mixtures.push_back(std::unique_ptr<GaussianMixture>(mixture));
		}
	});
}

void gmvis::pylib::Visualizer::set_gaussian_mixtures_from_paths(py::list paths)
{
	pushCommand([this, paths] {
		m_mixtures.clear();
		for (int i = 0; i < paths.size(); ++i) {
			auto newGM = DataLoader::readGMfromPLY(QString(paths[i].cast<std::string>().c_str()), false);
			if (newGM) {
				m_mixtures.push_back(std::move(newGM));
			}
		}
	});
}

void Visualizer::set_callback(py::object callback)
{
	pushCommand([this, callback] {
		m_callback = std::make_unique<py::object>(callback);
	});
}

py::array_t<float> Visualizer::render(int epoch)
{
	py::array_t<float> result = py::array_t<float>();
	if (m_async) {
		pushCommand([this, epoch] {
			processRenderRequest(epoch);
		});
	}
	else {
		result = processRenderRequest(epoch);
	}
	return result;
}

void Visualizer::finish()
{
	pushCommand([this] {
		cleanup();
		});
	if (m_thread) {
		m_thread->join();
		m_thread.reset();
	}
}

void Visualizer::forceStop()
{
	//delete all other commands
	m_commandrequests_mutex.lock();
	m_commandrequests = std::queue<std::function<void()>>();
	m_commandrequests_mutex.unlock();
	pushCommand([this] {
		cleanup();
		});
	if (m_thread) {
		m_thread->join();
		m_thread.reset();
	}
}


void Visualizer::pushCommand(std::function<void()> cmd)
{
	if (m_async) {
		m_commandrequests_mutex.lock();
		m_commandrequests.push(cmd);
		m_commandrequests_mutex.unlock();
	}
	else {
		cmd();
	}
}

//TODO
py::array_t<float> Visualizer::processRenderRequest(int epoch)
{
	float* retdata;
	size_t imagesize = m_surface->getWidth() * m_surface->getHeight() * 4;
	int famount = m_surface->isEllipsoidDisplayEnabled() + m_surface->isDensityDisplayEnabled();
	if (!m_async && !m_callback) {
		retdata = new float[imagesize * famount * m_mixtures.size()];
	}
	QElapsedTimer timer;
	timer.start();
	for (int i = 0; i < m_mixtures.size(); ++i) {
		if (m_pointclouds.size() > 0) {
			m_surface->setPointcloud(m_pointclouds[i].get());
		}
		if (m_cameraAuto) {
			calculateCameraPositionByPointcloud(i);
		}
		m_surface->setMixture(m_mixtures[i].get());
		std::vector<std::unique_ptr<Image>> pixeldata = m_surface->render();
		//Save!
		if (m_callback) {
			if (m_surface->isEllipsoidDisplayEnabled()) {
				(*m_callback)(epoch, pixeldata[0]->toNpArray(), i, 0);
			}
			if (m_surface->isDensityDisplayEnabled()) {
				(*m_callback)(epoch, pixeldata[pixeldata.size() == 2]->toNpArray(), i, 1);
			}
		}
		else if (!m_async) {
			if (m_surface->isEllipsoidDisplayEnabled()) {
				float* imgdata = pixeldata[0]->data();
				memcpy(&retdata[i * imagesize * famount], imgdata, sizeof(float) * imagesize);
			}
			if (m_surface->isDensityDisplayEnabled()) {
				int idx = (pixeldata.size() == 2);
				float* imgdata = pixeldata[idx]->data();
				memcpy(&retdata[i * imagesize * famount + idx * imagesize], imgdata, sizeof(float) * imagesize);
			}
		}
	}
	auto ms = timer.elapsed();
	pyprint("Visualizer: GM rendered. Time: " + std::to_string(ms) + "ms");
	if (!m_async && !m_callback) {
		int h = m_surface->getHeight();
		int w = m_surface->getWidth();
		int b = m_mixtures.size();
		return py::array_t<float>(
			{ b, famount, h, w, 4 },
			{ sizeof(float) * famount * h * w * 4, sizeof(float) * h * w * 4, sizeof(float) * w * 4, sizeof(float) * 4, sizeof(float) },
			retdata
			);
	}
	else {
		return py::array_t<float>();
	}
}

void Visualizer::visThread()
{
	pyprint("Visualizer: Thread started!");

	//As long as thread is still running
	while (m_threadrunning) {

		//Process current commands, until none are left
		m_commandrequests_mutex.lock();
		if (!m_commandrequests.empty()) {
			std::function<void()> func = m_commandrequests.front();
			m_commandrequests.pop();
			m_commandrequests_mutex.unlock();
			func();
		}
		else {
			m_commandrequests_mutex.unlock();
		}
	}
	pyprint("Visualizer: Thread stopped");
}

void Visualizer::calculateCameraPositionByPointcloud(int index)
{
	if (m_pointclouds.size() > index) {
		PointCloud* pc = m_pointclouds[index].get();
		QVector3D min = pc->getBBMin();
		QVector3D max = pc->getBBMax();
		QVector3D center = (min + max) / 2.0;
		QVector3D extend = (min - max) / 2.0;
		auto cam = m_surface->getCamera();
		cam->setTranslation(center);
		cam->setRadius(std::max(extend.x(), std::max(extend.y(), extend.z())) * 5);
		cam->setXRotation(45);
		cam->setYRotation(-135);
	}
}

void Visualizer::cleanup()
{
	m_application->exit();
	m_mixtures.clear();
	m_pointclouds.clear();
	m_surface.reset();
	m_application.reset();
	m_callback.reset();
	m_threadrunning = false;
}
