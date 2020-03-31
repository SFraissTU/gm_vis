#include "Image.h"

Image::Image(size_t width, size_t height, size_t channels)
{
	m_width = width;
	m_height = height;
	m_channels = channels;
	m_data = new float[channels * width * height];
}

Image::~Image()
{
	delete[] m_data;
}

float* Image::data()
{
	return m_data;
}

size_t Image::width() const
{
	return m_width;
}

size_t Image::height() const
{
	return m_height;
}

size_t Image::channels() const
{
	return m_channels;
}

//py::buffer_info Image::bufferInfo(Image& i)
//{
//	return py::buffer_info(
//		i.data(),
//		sizeof(float),
//		py::format_descriptor<float>::format(),
//		3,
//		{ i.m_height, i.m_width, i.m_channels},
//		{ sizeof(float) * i.m_channels * i.m_width, sizeof(float) * i.m_channels, sizeof(float) }
//	);
//}

py::array_t<float> Image::toNpArray()
{
	return py::array_t<float>(
		{ m_height, m_width, m_channels },
		{ sizeof(float) * m_channels * m_width, sizeof(float) * m_channels, sizeof(float) },
		m_data
	);
}
