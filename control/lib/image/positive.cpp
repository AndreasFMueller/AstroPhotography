/*
 * positive.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller
 */
#include <AstroImage.h>

namespace astro {
namespace image {
namespace ops {

#define positiveit(image, Pixel)					\
{									\
	Image<Pixel >	*img = dynamic_cast<Image<Pixel>*>(&*image);	\
	if (NULL != img) {						\
		img->positive();					\
		return;							\
	}								\
}

void	positive(ImagePtr image) {
	positiveit(image, float)
	positiveit(image, double)
	throw std::runtime_error("don't know how to positive image");
}

} // namespace ops
} // namespace image
} // namespace astro
