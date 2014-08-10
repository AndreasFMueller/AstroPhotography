/*
 * Adapters.cpp -- Functions extending adapters to ImagePtr
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroAdapter.h>

namespace astro {
namespace adapter {

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

#define type_convert_typed(image, Pixel)				\
{									\
	Image<Pixel >	*imageptr					\
		= dynamic_cast<Image<Pixel > *>(&*image);		\
	if (NULL != imageptr) {						\
		return new TypeConversionAdapter<double, Pixel >(*imageptr);\
	}								\
}

static ConstImageAdapter<double>	*type_convert(const ImagePtr image) {
	type_convert_typed(image, unsigned char);
	type_convert_typed(image, unsigned short);
	type_convert_typed(image, unsigned int);
	type_convert_typed(image, unsigned long);
	type_convert_typed(image, float);
	type_convert_typed(image, double);
	throw std::runtime_error("cannot convert this image to double pixel");
}

DoubleAdapter::DoubleAdapter(const ImagePtr image) :
	ConstImageAdapter<double>(image->size()), _image(image) {
	ConstImageAdapter<double>	*adapter = type_convert(image);
	doubleimage = std::shared_ptr<ConstImageAdapter<double> >(adapter);
}

} // namespace image
} // namespace astro
