/*
 * HaarWavelet.cpp
 *
 * (c) 2023 Prof Dr Andreas Müller
 */
#include <AstroImage.h>
#include <AstroWavelets.h>
#include <AstroDebug.h>

namespace astro {
namespace adapter {

#define	do_wavelettransform(image, inverse, Pixel)			\
	{								\
		Image<Pixel>	*imagep					\
			= dynamic_cast<Image<Pixel >*>(&*image);	\
		if (NULL != imagep) {					\
			return haarwavelettransform(*imagep, inverse);	\
		}							\
	}

ImagePtr	haarwavelettransform(ImagePtr image, bool inverse) {
	do_wavelettransform(image, inverse, unsigned char);
	do_wavelettransform(image, inverse, unsigned short);
	do_wavelettransform(image, inverse, unsigned int);
	do_wavelettransform(image, inverse, unsigned long);
	do_wavelettransform(image, inverse, float);
	do_wavelettransform(image, inverse, double);
	do_wavelettransform(image, inverse, RGB<unsigned char>);
	do_wavelettransform(image, inverse, RGB<unsigned short>);
	do_wavelettransform(image, inverse, RGB<unsigned int>);
	do_wavelettransform(image, inverse, RGB<unsigned long>);
	do_wavelettransform(image, inverse, RGB<float>);
	do_wavelettransform(image, inverse, RGB<double>);
	throw std::runtime_error("cannot wavelet transform this iamge type");
}

} // namespace adapter
} // namespace astro
