/*
 * SmallConvolve.cpp -- convolution with small images
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConvolve.h>
#include <AstroAdapter.h>
#include <AstroDebug.h>
#include <AstroFormat.h>

namespace astro {
namespace image {

/**
 * \brief template function to perform the convolution
 *
 * \param small		the small image to convolve, typically the PSF
 * \param image		the image to convolve
 */
template<typename Pixel>
ImagePtr	convolve(const ConstImageAdapter<double>& small,
			ImagePtr image) {
	Image<Pixel >	*img = dynamic_cast<Image<Pixel > *>(&*image);
	if (NULL == img) {
		std::string	msg = stringprintf("pixel type %s does not "
			"match image", demangle_cstr(Pixel()));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "convolving a %s %s with a %s %s",
		small.getSize().toString().c_str(),
		demangle_cstr(small),
		image->size().toString().c_str(),
		demangle_cstr(*img));
	adapter::ConvolutionAdapter<Pixel >	ca(*img, small);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "perform the convolution");
	return ImagePtr(new Image<Pixel >(ca));
}

#define convolve_typed(small, image, Pixel)				\
try {									\
	return convolve<Pixel>(small, image);				\
} catch (const std::exception& x) {					\
	if (false) {							\
		debug(LOG_DEBUG, DEBUG_LOG, 0, "exception: %s", x.what());\
	}								\
}

/**
 * \brief Convolution with a small image
 *
 * \param small		the small image to convolute with
 * \param image		the image to convolve with small
 */
ImagePtr	smallConvolve(const ConstImageAdapter<double>& small,
			ImagePtr image) {
	// construct a convolution adapter from the image
	convolve_typed(small, image, unsigned char)
	convolve_typed(small, image, unsigned short)
	convolve_typed(small, image, unsigned int)
	convolve_typed(small, image, unsigned long)
	convolve_typed(small, image, float)
	convolve_typed(small, image, double)
	convolve_typed(small, image, RGB<unsigned char>)
	convolve_typed(small, image, RGB<unsigned short>)
	convolve_typed(small, image, RGB<unsigned int>)
	convolve_typed(small, image, RGB<unsigned long>)
	convolve_typed(small, image, RGB<float>)
	convolve_typed(small, image, RGB<double>)
	convolve_typed(small, image, YUYV<unsigned char>)
	convolve_typed(small, image, YUYV<unsigned short>)
	convolve_typed(small, image, YUYV<unsigned int>)
	convolve_typed(small, image, YUYV<unsigned long>)
	convolve_typed(small, image, YUYV<float>)
	convolve_typed(small, image, YUYV<double>)
	// if we get to this point, we were not able to convolute because
	// of an unknown Pixel type
	std::string	cause = stringprintf("unknown pixel type %s",
		demangle_cstr(*image));
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
	throw std::runtime_error(cause);
}

} // namespace image
} // namespace astro
