/*
 * Filters.cpp -- filters to compute filter values independent of pixel type
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroFilter.h>

using namespace astro::image;

namespace astro {
namespace image {
namespace filter {

#define	filter_typed(image, f, pixel)					\
	{								\
		Image<pixel>	*imagep					\
			= dynamic_cast<Image<pixel> *>(&*image);	\
		if (NULL != imagep) {					\
			f<pixel, double>	m;			\
			return m.filter(*imagep);			\
		}							\
	}

double	mean(const ImagePtr& image) {
	filter_typed(image, Mean, unsigned char);
	filter_typed(image, Mean, unsigned short);
	filter_typed(image, Mean, unsigned int);
	filter_typed(image, Mean, unsigned long);
	filter_typed(image, Mean, float);
	filter_typed(image, Mean, double);
	return 0;
}

#define	filter_typed1(image, f, pixel)				\
	{								\
		Image<pixel>	*imagep					\
			= dynamic_cast<Image<pixel> *>(&*image);	\
		if (NULL != imagep) {					\
			f<pixel, double>	m;			\
			return m.filter(*imagep);			\
		}							\
	}

double	median(const ImagePtr& image) {
	filter_typed1(image, Median, unsigned char);
	filter_typed1(image, Median, unsigned short);
	filter_typed1(image, Median, unsigned int);
	filter_typed1(image, Median, unsigned long);
	filter_typed1(image, Median, float);
	filter_typed1(image, Median, double);
	return 0;
}

double	max(const ImagePtr& image) {
	filter_typed1(image, Max, unsigned char);
	filter_typed1(image, Max, unsigned short);
	filter_typed1(image, Max, unsigned int);
	filter_typed1(image, Max, unsigned long);
	filter_typed1(image, Max, float);
	filter_typed1(image, Max, double);
	return 0;
}

double	min(const ImagePtr& image) {
	filter_typed1(image, Min, unsigned char);
	filter_typed1(image, Min, unsigned short);
	filter_typed1(image, Min, unsigned int);
	filter_typed1(image, Min, unsigned long);
	filter_typed1(image, Min, float);
	filter_typed1(image, Min, double);
	return 0;
}

#define	filter_focusfom(image, pixel, diagonal)				\
	{								\
		Image<pixel>	*imagep					\
			= dynamic_cast<Image<pixel> *>(&*image);	\
		if (NULL != imagep) {					\
			astro::image::filter::FocusFOM<pixel>		\
				m(diagonal);				\
			return m.filter(*imagep);			\
		}							\
	}

double	focusFOM(const ImagePtr& image, const bool diagonal) {
	filter_focusfom(image, unsigned char, diagonal);
	filter_focusfom(image, unsigned short, diagonal);
	filter_focusfom(image, unsigned int, diagonal);
	filter_focusfom(image, unsigned long, diagonal);
	filter_focusfom(image, float, diagonal);
	filter_focusfom(image, double, diagonal);
	return 0;
}

#define	filter_mask(image, pixel, mf)					\
	{								\
		Image<pixel>	*imagep					\
			= dynamic_cast<Image<pixel> *>(&*image);	\
		if (NULL != imagep) {					\
			Mask<pixel>	m(mf);				\
			m(*imagep);					\
		}							\
	}

void	mask(MaskingFunction& maskingfunction, ImagePtr image) {
	filter_mask(image, unsigned char, maskingfunction);
	filter_mask(image, unsigned short, maskingfunction);
	filter_mask(image, unsigned int, maskingfunction);
	filter_mask(image, unsigned long, maskingfunction);
	filter_mask(image, float, maskingfunction);
	filter_mask(image, double, maskingfunction);
}

} // namespace filter
} // namespace image
} // namespace astro
