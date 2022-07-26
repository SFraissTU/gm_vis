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

void Image::clamp(std::vector<float> min, std::vector<float> max)
{
	for (int x = 0; x < m_width; x++) {
		for (int y = 0; y < m_height; y++) {
			for (int c = 0; c < m_channels; ++c) {
				m_data[y * m_channels * m_width + m_channels * x + c] = 
					std::clamp(m_data[y * m_channels * m_width + m_channels * x + c], min[c], max[c]);
			}
		}
	}
}

void Image::invertHeight()
{
	for (int y = 0; y < m_height / 2; y++) {
		for (int x = 0; x < m_width; x++) {
			for (int c = 0; c < m_channels; ++c) {
				float temp = m_data[y * m_channels * m_width + m_channels * x + c];
				m_data[y * m_channels * m_width + m_channels * x + c] = m_data[(m_height - 1 - y) * m_channels * m_width + m_channels * x + c];
				m_data[(m_height - 1 - y) * m_channels * m_width + m_channels * x + c] = temp;
			}
		}
	}
}

py::array_t<float> Image::toNpArray()
{
	return py::array_t<float>(
		{ m_height, m_width, m_channels },
		{ sizeof(float) * m_channels * m_width, sizeof(float) * m_channels, sizeof(float) },
		m_data
	);
}
