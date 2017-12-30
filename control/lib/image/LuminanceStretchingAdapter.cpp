/*
 * LuminanceStretchingAdapter.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTonemapping.h>

namespace astro {
namespace adapter {

#define do_stretch(Pixel, image, factor)				\
{									\
	Image<Pixel >	*imageptr					\
		= dynamic_cast<Image<Pixel >*>(&*image);		\
	if (NULL != imageptr) {						\
		ImagePtr	result(luminancestretching<Pixel>(	\
					*imageptr, factor));		\
		return result;						\
	}								\
}

ImagePtr	luminancestretching(ImagePtr image, LuminanceFactor& factor) {
	do_stretch(unsigned char, image, factor);
	do_stretch(unsigned short, image, factor);
	do_stretch(unsigned int, image, factor);
	do_stretch(unsigned long, image, factor);
	do_stretch(float, image, factor);
	do_stretch(double, image, factor);
	do_stretch(RGB<unsigned char>, image, factor);
	do_stretch(RGB<unsigned short>, image, factor);
	do_stretch(RGB<unsigned int>, image, factor);
	do_stretch(RGB<unsigned long>, image, factor);
	do_stretch(RGB<float>, image, factor);
	do_stretch(RGB<double>, image, factor);
	throw std::runtime_error("cannot luminance stretch this image");
}

} // namespace adapter
} // namespace astro
