/*
 * Adapters.cpp -- Functions extending adapters to ImagePtr
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroAdapter.h>

namespace astro {
namespace image {

#define	sample_type(Adapter, Pixel)					\
{									\
	Image<Pixel >	*imageptr					\
		= dynamic_cast<Image<Pixel> *>(&*image);		\
	if (NULL != imageptr) {						\
		Adapter<Pixel>	sa(*imageptr, sampling);\
		return ImagePtr(new Image<Pixel>(sa));			\
	}								\
}

ImagePtr	downsample(ImagePtr image, const ImageSize& sampling) {
	sample_type(DownSamplingAdapter, unsigned char);
	sample_type(DownSamplingAdapter, unsigned short);
	sample_type(DownSamplingAdapter, unsigned int);
	sample_type(DownSamplingAdapter, unsigned long);
	sample_type(DownSamplingAdapter, float);
	sample_type(DownSamplingAdapter, double);
	sample_type(DownSamplingAdapter, RGB<unsigned char>);
	sample_type(DownSamplingAdapter, RGB<unsigned short>);
	sample_type(DownSamplingAdapter, RGB<unsigned int>);
	sample_type(DownSamplingAdapter, RGB<unsigned long>);
	sample_type(DownSamplingAdapter, RGB<float>);
	sample_type(DownSamplingAdapter, RGB<double>);
	throw std::runtime_error("cannot downsample this image type");
}

ImagePtr	upsample(ImagePtr image, const ImageSize& sampling) {
	sample_type(UpSamplingAdapter, unsigned char);
	sample_type(UpSamplingAdapter, unsigned short);
	sample_type(UpSamplingAdapter, unsigned int);
	sample_type(UpSamplingAdapter, unsigned long);
	sample_type(UpSamplingAdapter, float);
	sample_type(UpSamplingAdapter, double);
	sample_type(UpSamplingAdapter, RGB<unsigned char>);
	sample_type(UpSamplingAdapter, RGB<unsigned short>);
	sample_type(UpSamplingAdapter, RGB<unsigned int>);
	sample_type(UpSamplingAdapter, RGB<unsigned long>);
	sample_type(UpSamplingAdapter, RGB<float>);
	sample_type(UpSamplingAdapter, RGB<double>);
	throw std::runtime_error("cannot upsample this image type");
}

} // namespace image
} // namespace astro
