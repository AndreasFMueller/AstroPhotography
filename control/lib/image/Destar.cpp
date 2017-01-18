/*
 * Destar.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroAdapter.h>

namespace astro {
namespace adapter {

#define destar_mono(Pixel, imageptr)					\
{									\
	Image<Pixel>	*image						\
		= dynamic_cast<Image<Pixel>*>(&*imageptr);		\
	if (NULL != image) {						\
		return ImagePtr(destar<Pixel>(*image, radius));		\
	}								\
}

#define destar_color(Pixel, imageptr)					\
{									\
	Image<RGB<Pixel> >	*image					\
		= dynamic_cast<Image<RGB<Pixel> >*>(&*imageptr);	\
	if (NULL != image) {						\
		LuminanceAdapter<RGB<Pixel>, float>    la(*image);	\
		Image<float>	*limage = new Image<float>(la);         \
		ImagePtr	limageptr(limage);			\
		return ImagePtr(destar<float>(*limage, radius));	\
	}								\
}

ImagePtr	destarptr(ImagePtr imageptr, int radius) {
	destar_mono(unsigned char, imageptr)
	destar_mono(unsigned short, imageptr)
	destar_mono(unsigned int, imageptr)
	destar_mono(unsigned long, imageptr)
	destar_mono(float, imageptr)
	destar_mono(double, imageptr)
	destar_color(unsigned char, imageptr)
	destar_color(unsigned short, imageptr)
	destar_color(unsigned int, imageptr)
	destar_color(unsigned long, imageptr)
	destar_color(float, imageptr)
	destar_color(double, imageptr)
	throw std::runtime_error("unknown pixel type");
}

} // namespace adapter
} // namespace astro
