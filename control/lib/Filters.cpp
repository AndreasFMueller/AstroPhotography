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

#define	filter_typed(image, f, pixel)				\
	{								\
		Image<pixel>	*imagep					\
			= dynamic_cast<Image<pixel> *>(&*image);	\
		if (NULL != imagep) {					\
			f<pixel, double>	m;			\
			return m.filter(*imagep);			\
		}							\
	}

double	mean(const ImagePtr& image) {
	filter_typed(image, astro::image::filter::Mean, unsigned char);
	filter_typed(image, astro::image::filter::Mean, unsigned short);
	filter_typed(image, astro::image::filter::Mean, unsigned int);
	filter_typed(image, astro::image::filter::Mean, unsigned long);
	filter_typed(image, astro::image::filter::Mean, float);
	filter_typed(image, astro::image::filter::Mean, double);
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
	filter_typed1(image, astro::image::filter::Median, unsigned char);
	filter_typed1(image, astro::image::filter::Median, unsigned short);
	filter_typed1(image, astro::image::filter::Median, unsigned int);
	filter_typed1(image, astro::image::filter::Median, unsigned long);
	filter_typed1(image, astro::image::filter::Median, float);
	filter_typed1(image, astro::image::filter::Median, double);
	return 0;
}

double	max(const ImagePtr& image) {
	filter_typed1(image, astro::image::filter::Max, unsigned char);
	filter_typed1(image, astro::image::filter::Max, unsigned short);
	filter_typed1(image, astro::image::filter::Max, unsigned int);
	filter_typed1(image, astro::image::filter::Max, unsigned long);
	filter_typed1(image, astro::image::filter::Max, float);
	filter_typed1(image, astro::image::filter::Max, double);
	return 0;
}

double	min(const ImagePtr& image) {
	filter_typed1(image, astro::image::filter::Min, unsigned char);
	filter_typed1(image, astro::image::filter::Min, unsigned short);
	filter_typed1(image, astro::image::filter::Min, unsigned int);
	filter_typed1(image, astro::image::filter::Min, unsigned long);
	filter_typed1(image, astro::image::filter::Min, float);
	filter_typed1(image, astro::image::filter::Min, double);
	return 0;
}



} // namespace filter
} // namespace image
} // namespace astro
