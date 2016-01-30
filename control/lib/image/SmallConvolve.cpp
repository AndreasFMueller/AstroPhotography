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

#define	convolve_typed(image, Pixel)					\
{									\
	Image<Pixel >	*img = dynamic_cast<Image<Pixel > *>(&*image);	\
	if (NULL != img) {						\
		adapter::ConvolutionAdapter<Pixel >	ca(*img, small);\
		return ImagePtr(new Image<Pixel >(ca));			\
	}								\
}

ImagePtr	smallConvolve(const ConstImageAdapter<double>& small,
			ImagePtr image) {
	convolve_typed(image, unsigned char)
	convolve_typed(image, unsigned short)
	convolve_typed(image, unsigned int)
	convolve_typed(image, unsigned long)
	convolve_typed(image, float)
	convolve_typed(image, double)
	convolve_typed(image, RGB<unsigned char>)
	convolve_typed(image, RGB<unsigned short>)
	convolve_typed(image, RGB<unsigned int>)
	convolve_typed(image, RGB<unsigned long>)
	convolve_typed(image, RGB<float>)
	convolve_typed(image, RGB<double>)
	convolve_typed(image, YUYV<unsigned char>)
	convolve_typed(image, YUYV<unsigned short>)
	convolve_typed(image, YUYV<unsigned int>)
	convolve_typed(image, YUYV<unsigned long>)
	convolve_typed(image, YUYV<float>)
	convolve_typed(image, YUYV<double>)
	std::string	cause = stringprintf("unknown pixel type %s",
		demangle(typeid(*image).name()).c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
	throw std::runtime_error(cause);
}

} // namespace image
} // namespace astro
