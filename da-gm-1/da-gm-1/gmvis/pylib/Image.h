#pragma once
#include <pybind11/pybind11.h>
namespace py = pybind11;

class Image {
public:

	Image(size_t width, size_t height, size_t channels);
	~Image();
	float* data();
	size_t width() const;
	size_t height() const;
	size_t channels() const;

	static py::buffer_info bufferInfo(Image& i);

private:
	size_t m_width;
	size_t m_height;
	size_t m_channels;
	float* m_data;
};