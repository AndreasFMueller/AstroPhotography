/*
 * LogImage.cpp -- log scaling of images
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroAdapter.h>

namespace astro {
namespace adapter {

#define	do_rgblog(image, T, S)						\
	{								\
		Image<RGB<T> >	*imagep					\
			= dynamic_cast<Image<RGB<T> >*>(&*image);	\
		if (NULL != imagep) {					\
			return RGBLogAdapter<T,S>::logimage(*imagep);	\
		}							\
	}

ImagePtr	rgblogimage(ImagePtr image) {
	do_rgblog(image, float, float)
	do_rgblog(image, double, double)
	throw std::runtime_error("cannot log image with this pixel type");
}

#define	do_logimage(image, Pixel)					\
	{								\
		Image<Pixel>	*imagep					\
			= dynamic_cast<Image<Pixel>*>(&*image);		\
		if (NULL != imagep) {					\
			return LogAdapter<Pixel>::logimage(*imagep);	\
		}							\
	}

ImagePtr	monologimage(ImagePtr image) {
	do_logimage(image, float)
	do_logimage(image, double)
	throw std::runtime_error("cannot log image with this pixel type");
}

ImagePtr	logimage(ImagePtr image) {
	if (3 == image->planes()) {
		return rgblogimage(image);
	} else {
		return monologimage(image);
	}
}

} // namespace adapter
} // namespace astro
