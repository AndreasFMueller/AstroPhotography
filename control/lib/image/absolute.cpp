/*
 * absolute.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller
 */
#include <AstroImage.h>

namespace astro {
namespace image {
namespace ops {

#define absoluteit(image, Pixel)					\
{									\
	Image<Pixel >	*img = dynamic_cast<Image<Pixel>*>(&*image);	\
	if (NULL != img) {						\
		img->absolute();					\
		return;							\
	}								\
}

void	absolute(ImagePtr image) {
	absoluteit(image, float)
	absoluteit(image, double)
	throw std::runtime_error("don't know how to absolute image");
}

} // namespace ops
} // namespace image
} // namespace astro
