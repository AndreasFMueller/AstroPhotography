/*
 * ColorScaling.cpp -- operator function to change colors
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroOperators.h>

namespace astro {
namespace image {
namespace operators {

#define	do_colorscaling(image, scale, Pixel)				\
	{								\
		Image<RGB<Pixel> >	*imagep				\
			= dynamic_cast<Image<RGB<Pixel> >*>(&*image);	\
		if (NULL != imagep) {					\
			ColorScalingOperator<Pixel> colorscaling(scale);\
			colorscaling(*imagep);				\
		}							\
	}

void	colorscaling_operator(const RGB<double>& scale, ImagePtr image) {
	do_colorscaling(image, scale, unsigned char);
	do_colorscaling(image, scale, unsigned short);
	do_colorscaling(image, scale, unsigned int);
	do_colorscaling(image, scale, unsigned long);
	do_colorscaling(image, scale, float);
	do_colorscaling(image, scale, double);
	throw std::runtime_error("cannot color scale this type of image");
}

} // namespace operators
} // namespace image
} // namespace astro
