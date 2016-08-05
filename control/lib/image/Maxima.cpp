/*
 * Maxima.cpp -- Compute a list of maxima
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroImage.h>
#include <AstroFilter.h>

namespace astro {
namespace image {

template<typename Pixel>
std::list<ImagePoint>	Maxima(const Image<Pixel>& image, unsigned long limit) {
	// first compute the maximum value of the image
	filter::Max<Pixel, Pixel>	m;
	Pixel	maximum = m(image);

	// prepare the result list
	std::list<ImagePoint>	maxima;

	// now go through the image and check for pixels that have the
	// same value
	int	w = image.getSize().width();
	int	h = image.getSize().height();
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			if (maximum == image.pixel(x, y)) {
				maxima.push_back(ImagePoint(x, y));
				if (maxima.size() > limit) {
					throw std::range_error("too many maxima");
				}
			}
		}
	}

	return maxima;
}

#define findmaximum(image, limit, Pixel)				\
{									\
	Image<Pixel>	*imgptr = dynamic_cast<Image<Pixel>*>(&*image);	\
	if (NULL == imgptr) {						\
		return Maxima<Pixel>(*imgptr, limit);			\
	}								\
}

std::list<ImagePoint>	Maxima(ImagePtr image, unsigned long limit) {
	findmaximum(image, limit, unsigned char)
	findmaximum(image, limit, unsigned short)
	findmaximum(image, limit, unsigned int)
	findmaximum(image, limit, unsigned long)
	findmaximum(image, limit, float)
	findmaximum(image, limit, double)
	return std::list<ImagePoint>();
}

} // namespace image
} // namespace astro
