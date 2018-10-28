/*
 * FormatReductionRGB.cpp
 *
 * (c) 2018 prof Dr Andreas Müller, Hochschule Rapeprswil
 */
#include <AstroImage.h>
#include <AstroFilterfunc.h>

namespace astro {
namespace image {

FormatReductionRGB::FormatReductionRGB(const ImageSize& size,
	double min, double max)
	: FormatReductionBase(min, max),
	  ConstImageAdapter<RGB<unsigned char> >(size) {
}

template<typename Pixel>
class FormatReductionRGBImpl : public FormatReductionRGB {
	const ConstImageAdapter<RGB<Pixel> >&	_image;
public:
	FormatReductionRGBImpl(ConstImageAdapter<RGB<Pixel> >& image,
		double min, double max)
		: FormatReductionRGB(image.getSize(), min, max), _image(image) {
	}
	virtual RGB<unsigned char>	pixel(int x, int y) const {
		RGB<Pixel>	v = _image.pixel(x, y);
		return RGB<unsigned char>(clamp(v.R), clamp(v.G), clamp(v.B));
	}
};

#define	integer_reduce(image, pixel)					\
{									\
	Image<RGB<pixel> > *img = dynamic_cast<Image<RGB<pixel> >*>(&*image);\
	if (NULL != img) {						\
		return new FormatReductionRGBImpl<pixel>(*img,		\
			std::numeric_limits<pixel>::min(),		\
			std::numeric_limits<pixel>::max());		\
	}								\
}

#define	float_reduce(image, pixel)					\
{									\
	Image<RGB<pixel> > *img = dynamic_cast<Image<RGB<pixel> >*>(&*image);\
	if (NULL != img) {						\
		double	min = filter::min_RGB(image);			\
		double	max = filter::max_RGB(image);			\
		if (min == max) {					\
			max = min + 1;					\
		}							\
		return new FormatReductionRGBImpl<pixel>(*img,		\
			min, max);					\
	}								\
}

FormatReductionRGB	*FormatReductionRGB::get(ImagePtr image) {
	integer_reduce(image, unsigned char);
	integer_reduce(image, unsigned short);
	integer_reduce(image, unsigned int);
	integer_reduce(image, unsigned long);
	float_reduce(image, float);
	float_reduce(image, double);
	debug(LOG_ERR, DEBUG_LOG, 0, "cannot reduce %s", image->info().c_str());
	return NULL;
}

#define reduce(image, pixel, min, max)					\
{									\
	Image<RGB<pixel> > *img = dynamic_cast<Image<RGB<pixel> >*>(&*image);\
	if (NULL != img) {						\
		return new FormatReductionRGBImpl<pixel>(*img,		\
			min, max);					\
	}								\
}

FormatReductionRGB	*FormatReductionRGB::get(ImagePtr image,
		double min, double max) {
	reduce(image, unsigned char, min, max)
	reduce(image, unsigned short, min, max)
	reduce(image, unsigned int, min, max)
	reduce(image, unsigned long, min, max)
	reduce(image, float, min, max)
	reduce(image, double, min, max)
	debug(LOG_ERR, DEBUG_LOG, 0, "cannot reduce %s", image->info().c_str());
	return NULL;
}

FormatReductionRGB	*FormatReductionRGB::get(ImagePtr image,
		std::pair<double, double>& minmax) {
	return get(image, minmax.first, minmax.second);
}

} // namespace image
} // namespace astro
