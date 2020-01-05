/*
 * negative.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller
 */
#include <AstroImage.h>

namespace astro {
namespace image {
namespace ops {

#define negativeit(image, Pixel)					\
{									\
	Image<Pixel >	*img = dynamic_cast<Image<Pixel>*>(&*image);	\
	if (NULL != img) {						\
		img->negative();					\
		return;							\
	}								\
}

void	negative(ImagePtr image) {
	negativeit(image, float)
	negativeit(image, double)
	throw std::runtime_error("don't know how to negative image");
}

} // namespace ops
} // namespace image
} // namespace astro
