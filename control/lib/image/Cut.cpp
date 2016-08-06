/*
 * Cut.cpp -- cat a rectangle from an image
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroImageops.h>
#include <AstroAdapter.h>

namespace astro {
namespace image {
namespace ops {

#define	cutit(source, rectangle, Pixel)					\
{									\
	Image<Pixel>	*imgptr = dynamic_cast<Image<Pixel >*>(&*source);\
	if (NULL != imgptr) {						\
		Image<Pixel>	*target = cut(*imgptr, rectangle);	\
		return ImagePtr(target);				\
	}								\
}

ImagePtr	cut(ImagePtr source, const ImageRectangle& rectangle) {
	cutit(source, rectangle, unsigned char);
	cutit(source, rectangle, unsigned short);
	cutit(source, rectangle, unsigned int);
	cutit(source, rectangle, unsigned long);
	cutit(source, rectangle, float);
	cutit(source, rectangle, double);
	cutit(source, rectangle, RGB<unsigned char>);
	cutit(source, rectangle, RGB<unsigned short>);
	cutit(source, rectangle, RGB<unsigned int>);
	cutit(source, rectangle, RGB<unsigned long>);
	cutit(source, rectangle, RGB<float>);
	cutit(source, rectangle, RGB<double>);
	throw std::runtime_error("unknown pixel type for cut operation");
}

} // namespace ops
} // namespace image
} // namespace astro
