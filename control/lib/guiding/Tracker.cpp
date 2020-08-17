/*
 * Tracker.cpp -- tracker base class methods implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <AstroUtils.h>

using namespace astro::adapter;

namespace astro {
namespace guiding {

#define	construct(Pixel, newimage)					\
{									\
	Image<Pixel>	*imagep						\
		= dynamic_cast<Image<Pixel > *>(&*newimage);		\
	if (NULL != imagep) {						\
		return new LuminanceAdapter<Pixel, double>(*imagep);	\
	}								\
}

/**
 * \brief Construct a luminance adapter for the image
 *
 * All the phase correlation trackers operate on the luminance channel only,
 * so we provide this method in the base class to extract the luminance in
 * double format independently of the pixel type provided by the camera.
 */
ConstImageAdapter<double>	*Tracker::adapter(const ImagePtr image) {
	construct(unsigned char, image);
	construct(unsigned short, image);
	construct(unsigned int, image);
	construct(unsigned long, image);
	construct(float, image);
	construct(double, image);
	construct(RGB<unsigned char>, image);
	construct(RGB<unsigned short>, image);
	construct(RGB<unsigned int>, image);
	construct(RGB<unsigned long>, image);
	construct(RGB<float>, image);
	construct(RGB<double>, image);
	construct(YUYV<unsigned char>, image);
	construct(YUYV<unsigned short>, image);
	construct(YUYV<unsigned int>, image);
	construct(YUYV<unsigned long>, image);
	construct(YUYV<float>, image);
	construct(YUYV<double>, image);
	throw std::runtime_error("cannot track this image type");
}

std::string	Tracker::toString() const {
	return demangle_string(this);
}

} // namespace guiding
} // namespace astro
