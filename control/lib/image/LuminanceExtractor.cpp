/*
 * LuminanceExtractor.cpp
 *
 * (c) 2019 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroAdapter.h>

namespace astro {
namespace adapter {

#define construct_luminance(image, Pixel)				\
	{								\
	Image<Pixel >	*imagep						\
		= dynamic_cast<Image<Pixel >*>(&*image);		\
	if (NULL != imagep) {						\
		_luminance = new LuminanceAdapter<Pixel, double>(*imagep);\
		return;							\
	}								\
}

LuminanceExtractor::LuminanceExtractor(ImagePtr image)
	: ConstImageAdapter<double>(image->size()) {
	construct_luminance(image, unsigned char);
	construct_luminance(image, unsigned short);
	construct_luminance(image, unsigned int);
	construct_luminance(image, unsigned long);
	construct_luminance(image, float);
	construct_luminance(image, double);
	construct_luminance(image, RGB<unsigned char>);
	construct_luminance(image, RGB<unsigned short>);
	construct_luminance(image, RGB<unsigned int>);
	construct_luminance(image, RGB<unsigned long>);
	construct_luminance(image, RGB<float>);
	construct_luminance(image, RGB<double>);
#if 0
	construct_luminance(image, YUV<unsigned char>);
	construct_luminance(image, YUV<unsigned short>);
	construct_luminance(image, YUV<unsigned int>);
	construct_luminance(image, YUV<unsigned long>);
	construct_luminance(image, YUV<float>);
	construct_luminance(image, YUV<double>);
#endif
}

LuminanceExtractor::~LuminanceExtractor() {
	delete _luminance;
}

} // namespace adapter
} // namespace astro
