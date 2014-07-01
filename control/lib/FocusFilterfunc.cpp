/*
 * FocusFilterfunc.cpp -- focus filter functions
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroFilterfunc.h>
#include <AstroFilter.h>
#include <AstroAdapter.h>

using namespace astro::adapter;

namespace astro {
namespace image {
namespace filter {

#define	filter_typed(adapter, image, pixel)				\
{									\
	Image<pixel >	*imagep						\
		= dynamic_cast<Image<pixel > *>(&*image);		\
	if (NULL != imagep) {						\
		Sum<double>	sum;					\
		adapter<pixel>	ba(*imagep);				\
		return sum.filter(ba);					\
	}								\
}

double	focus_horizontalgradient(const ImagePtr& image) {
	filter_typed(HorizontalGradientAdapter, image, unsigned char);
	filter_typed(HorizontalGradientAdapter, image, unsigned short);
	filter_typed(HorizontalGradientAdapter, image, unsigned int);
	filter_typed(HorizontalGradientAdapter, image, unsigned long);
	filter_typed(HorizontalGradientAdapter, image, float);
	filter_typed(HorizontalGradientAdapter, image, double);
	return 0;
}

double	focus_verticalgradient(const ImagePtr& image) {
	filter_typed(VerticalGradientAdapter, image, unsigned char);
	filter_typed(VerticalGradientAdapter, image, unsigned short);
	filter_typed(VerticalGradientAdapter, image, unsigned int);
	filter_typed(VerticalGradientAdapter, image, unsigned long);
	filter_typed(VerticalGradientAdapter, image, float);
	filter_typed(VerticalGradientAdapter, image, double);
	return 0;
}

double	focus_squaredgradient(const ImagePtr& image) {
	filter_typed(SquaredGradientAdapter, image, unsigned char);
	filter_typed(SquaredGradientAdapter, image, unsigned short);
	filter_typed(SquaredGradientAdapter, image, unsigned int);
	filter_typed(SquaredGradientAdapter, image, unsigned long);
	filter_typed(SquaredGradientAdapter, image, float);
	filter_typed(SquaredGradientAdapter, image, double);
	return 0;
}

double	focus_horizontalbrenner(const ImagePtr& image) {
	filter_typed(HorizontalBrennerAdapter, image, unsigned char);
	filter_typed(HorizontalBrennerAdapter, image, unsigned short);
	filter_typed(HorizontalBrennerAdapter, image, unsigned int);
	filter_typed(HorizontalBrennerAdapter, image, unsigned long);
	filter_typed(HorizontalBrennerAdapter, image, float);
	filter_typed(HorizontalBrennerAdapter, image, double);
	return 0;
}

double	focus_verticalbrenner(const ImagePtr& image) {
	filter_typed(VerticalBrennerAdapter, image, unsigned char);
	filter_typed(VerticalBrennerAdapter, image, unsigned short);
	filter_typed(VerticalBrennerAdapter, image, unsigned int);
	filter_typed(VerticalBrennerAdapter, image, unsigned long);
	filter_typed(VerticalBrennerAdapter, image, float);
	filter_typed(VerticalBrennerAdapter, image, double);
	return 0;
}

double	focus_squaredbrenner(const ImagePtr& image) {
	filter_typed(BrennerAdapter, image, unsigned char);
	filter_typed(BrennerAdapter, image, unsigned short);
	filter_typed(BrennerAdapter, image, unsigned int);
	filter_typed(BrennerAdapter, image, unsigned long);
	filter_typed(BrennerAdapter, image, float);
	filter_typed(BrennerAdapter, image, double);
	return 0;
}

} // namespace filter
} // namespace image
} // namespace astro
