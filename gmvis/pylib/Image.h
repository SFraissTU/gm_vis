
#include "pyimport.h"

class Image {
public:

	Image(size_t width, size_t height, size_t channels);
	~Image();
	float* data();
	size_t width() const;
	size_t height() const;
	size_t channels() const;

	void clamp(std::vector<float> min, std::vector<float> max);
	void invertHeight();

	py::array_t<float> toNpArray();

private:
	size_t m_width;
	size_t m_height;
	size_t m_channels;
	float* m_data;
};