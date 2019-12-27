/*
 * GammaTransform.cpp
 *
 * (c) 2019 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroAdapter.h>

namespace astro {
namespace adapter {

#define do_gamma(image, settings, Pixel)				\
{									\
	Image<Pixel >	*imagep	= dynamic_cast<Image<Pixel >*>(&*image);\
	if (NULL != imagep) {						\
		return GammaTransformAdapter<Pixel >::corrected(*imagep,\
			settings);					\
	}								\
}

ImagePtr	gammatransform(ImagePtr image,
			const GammaTransformBase& gammasettings) {
	do_gamma(image, gammasettings, unsigned char)
	do_gamma(image, gammasettings, unsigned short)
	do_gamma(image, gammasettings, unsigned int)
	do_gamma(image, gammasettings, unsigned long)
	do_gamma(image, gammasettings, float)
	do_gamma(image, gammasettings, double)
	do_gamma(image, gammasettings, RGB<unsigned char>)
	do_gamma(image, gammasettings, RGB<unsigned short>)
	do_gamma(image, gammasettings, RGB<unsigned int>)
	do_gamma(image, gammasettings, RGB<unsigned long>)
	do_gamma(image, gammasettings, RGB<float>)
	do_gamma(image, gammasettings, RGB<double>)
	throw std::runtime_error("cannot gamma correct this type of pixel");
}

} // namespace adpater
} // namespace astro
