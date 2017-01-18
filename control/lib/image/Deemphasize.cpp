/*
 * Deemphasize.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroAdapter.h>

namespace astro {
namespace adapter {

#define do_deemphasize(Pixel, imageptr, blurredmask, degree)		\
{									\
	Image<Pixel>	*image = dynamic_cast<Image<Pixel>*>(&*imageptr);\
	if (image != NULL) {						\
		DeemphasizingAdapter<Pixel, double>	demph(*image,	\
			blurredmask, degree);				\
		return ImagePtr(new Image<Pixel>(demph));		\
	}								\
}

ImagePtr	deemphasize(ImagePtr imageptr,
	const ConstImageAdapter<double>& blurredmask, double degree) {
	do_deemphasize(unsigned char, imageptr, blurredmask, degree);
	do_deemphasize(unsigned short, imageptr, blurredmask, degree);
	do_deemphasize(unsigned int, imageptr, blurredmask, degree);
	do_deemphasize(unsigned long, imageptr, blurredmask, degree);
	do_deemphasize(float, imageptr, blurredmask, degree);
	do_deemphasize(double, imageptr, blurredmask, degree);
	do_deemphasize(RGB<unsigned char>, imageptr, blurredmask, degree);
	do_deemphasize(RGB<unsigned short>, imageptr, blurredmask, degree);
	do_deemphasize(RGB<unsigned int>, imageptr, blurredmask, degree);
	do_deemphasize(RGB<unsigned long>, imageptr, blurredmask, degree);
	do_deemphasize(RGB<float>, imageptr, blurredmask, degree);
	do_deemphasize(RGB<double>, imageptr, blurredmask, degree);
	throw std::runtime_error("don't know how to deemphasize this image");
}

} // namespace adapter
} // namespace astro
