/*
 * Flip.cpp -- implementation of flip operator
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroOperators.h>
#include <AstroDebug.h>
#include <AstroFormat.h>

namespace astro {
namespace image {
namespace operators {

#define	do_flip(image, pixel)						\
	{								\
		Image<pixel >	*imagep					\
			= dynamic_cast<Image<pixel >*>(&*image);	\
		if (NULL != imagep) {					\
			flip(*imagep);					\
			return;						\
		}							\
	}

void	flip(ImagePtr image) {
	if (!image) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no image to flip");
		return;
	}
	do_flip(image, unsigned char)
	do_flip(image, unsigned short)
	do_flip(image, unsigned int)
	do_flip(image, unsigned long)
	do_flip(image, float)
	do_flip(image, double)
	do_flip(image, RGB<unsigned char>)
	do_flip(image, RGB<unsigned short>)
	do_flip(image, RGB<unsigned int>)
	do_flip(image, RGB<unsigned long>)
	do_flip(image, RGB<float>)
	do_flip(image, RGB<double>)
	do_flip(image, YUYV<unsigned char>)
	do_flip(image, YUYV<unsigned short>)
	do_flip(image, YUYV<unsigned int>)
	do_flip(image, YUYV<unsigned long>)
	do_flip(image, YUYV<float>)
	do_flip(image, YUYV<double>)
	std::string	cause = stringprintf("unknown image type %s, cannot flip", demangle(image->pixel_type().name()).c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
	throw std::runtime_error(cause);
}


#define	do_hflip(image, pixel)						\
	{								\
		Image<pixel >	*imagep					\
			= dynamic_cast<Image<pixel >*>(&*image);	\
		if (NULL != imagep) {					\
			hflip(*imagep);					\
			return;						\
		}							\
	}

void	hflip(ImagePtr image) {
	if (!image) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no image to flip");
		return;
	}
	do_hflip(image, unsigned char)
	do_hflip(image, unsigned short)
	do_hflip(image, unsigned int)
	do_hflip(image, unsigned long)
	do_hflip(image, float)
	do_hflip(image, double)
	do_hflip(image, RGB<unsigned char>)
	do_hflip(image, RGB<unsigned short>)
	do_hflip(image, RGB<unsigned int>)
	do_hflip(image, RGB<unsigned long>)
	do_hflip(image, RGB<float>)
	do_hflip(image, RGB<double>)
	do_hflip(image, YUYV<unsigned char>)
	do_hflip(image, YUYV<unsigned short>)
	do_hflip(image, YUYV<unsigned int>)
	do_hflip(image, YUYV<unsigned long>)
	do_hflip(image, YUYV<float>)
	do_hflip(image, YUYV<double>)
	std::string	cause = stringprintf("unknown image type %s, cannot hflip", demangle(image->pixel_type().name()).c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
	throw std::runtime_error(cause);
}

} // namespace operators
} // namespace image
} // namespace astro
