/*
 * FormatReduction.cpp
 *
 * (c) 2018 prof Dr Andreas Müller, Hochschule Rapeprswil
 */
#include <AstroImage.h>
#include <AstroFilterfunc.h>

namespace astro {
namespace image {

FormatReduction::FormatReduction(const ImageSize& size, double min, double max)
	: FormatReductionBase(min, max),
	  ConstImageAdapter<unsigned char>(size) {
}

template<typename Pixel>
class FormatReductionImpl : public FormatReduction {
	const ConstImageAdapter<Pixel >&	_image;
public:
	FormatReductionImpl(ConstImageAdapter<Pixel >& image,
		double min, double max)
		: FormatReduction(image.getSize(), min, max), _image(image) {
	}
	virtual unsigned char	pixel(int x, int y) const {
		return clamp(_image.pixel(x, y));
	}
};

#define	integer_reduce(image, pixel)					\
{									\
	Image<pixel >	*img = dynamic_cast<Image<pixel >*>(&*image);	\
	if (NULL != img) {						\
		return new FormatReductionImpl<pixel>(*img, 		\
			std::numeric_limits<pixel>::min(),		\
			std::numeric_limits<pixel>::max());		\
	}								\
}

#define float_reduce(image, pixel)					\
{									\
	Image<pixel >	*img = dynamic_cast<Image<pixel >*>(&*image);	\
	if (NULL != img) {						\
		double	min = filter::min(image);			\
		double	max = filter::max(image);			\
		if (max == min) {					\
			max = min + 1;					\
		}							\
		return new FormatReductionImpl<pixel>(*img, min, max);	\
	}								\
}

FormatReduction	*FormatReduction::get(ImagePtr image) {
	integer_reduce(image, unsigned char)
	integer_reduce(image, unsigned short)
	integer_reduce(image, unsigned int)
	integer_reduce(image, unsigned long)
	float_reduce(image, float)
	float_reduce(image, double)
	debug(LOG_ERR, DEBUG_LOG, 0, "cannot reduce %s", image->info().c_str());
	return NULL;
}

#define	reduce(image, pixel, min, max)					\
{									\
	Image<pixel>	*img = dynamic_cast<Image<pixel>*>(&*image);	\
	if (NULL != img) {						\
		return new FormatReductionImpl<pixel>(*img, min, max);	\
	}								\
}

FormatReduction	*FormatReduction::get(ImagePtr image, double min, double max) {
	reduce(image, unsigned char, min, max)
	reduce(image, unsigned short, min, max)
	reduce(image, unsigned int, min, max)
	reduce(image, unsigned long, min, max)
	reduce(image, float, min, max)
	reduce(image, double, min, max)
	debug(LOG_ERR, DEBUG_LOG, 0, "cannot reduct %s", image->info().c_str());
	return NULL;
}

FormatReduction	*FormatReduction::get(ImagePtr image,
			std::pair<double, double>&minmax) {
	return get(image, minmax.first, minmax.second);
}

} // namespace image
} // namespace astro
