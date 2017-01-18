/*
 * Luminance.cpp -- filter to convert an image into a luminance image
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroAdapter.h>
#include <AstroDebug.h>

namespace astro {
namespace adapter {

#define do_luminance(image, Pixel, T)					\
{									\
	Image<Pixel>	*imagep = dynamic_cast<Image<Pixel >*>(&*image);\
	if (NULL != imagep) {						\
		return ImagePtr(adapter::luminance<Pixel, T>(*imagep));	\
	}								\
}

ImagePtr	luminanceptr(ImagePtr image) {
	do_luminance(image, unsigned char, unsigned char)
	do_luminance(image, unsigned short, unsigned short)
	do_luminance(image, unsigned int, unsigned int)
	do_luminance(image, unsigned long, unsigned long)
	do_luminance(image, float, float)
	do_luminance(image, double, double)
	do_luminance(image, RGB<unsigned char>, unsigned char)
	do_luminance(image, RGB<unsigned short>, unsigned short)
	do_luminance(image, RGB<unsigned int>, unsigned int)
	do_luminance(image, RGB<unsigned long>, unsigned long)
	do_luminance(image, RGB<float>, float)
	do_luminance(image, RGB<double>, double)
	std::string	msg = stringprintf("cannot get luminance for %s pixels",
		demangle(image->pixel_type().name()).c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

} // namespace adapter
} // namespace astro
