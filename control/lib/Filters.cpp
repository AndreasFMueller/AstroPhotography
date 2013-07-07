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

#define	filter_typed(image, f, pixel, grid)				\
	{								\
		Image<pixel>	*imagep					\
			= dynamic_cast<Image<pixel> *>(&*image);	\
		if (NULL != imagep) {					\
			f<pixel, double>	m;			\
			return m.filter(*imagep, grid);			\
		}							\
	}

double	mean(const ImagePtr& image, const Subgrid grid) {
	filter_typed(image, astro::image::filter::Mean, unsigned char, grid);
	filter_typed(image, astro::image::filter::Mean, unsigned short, grid);
	filter_typed(image, astro::image::filter::Mean, unsigned int, grid);
	filter_typed(image, astro::image::filter::Mean, unsigned long, grid);
	filter_typed(image, astro::image::filter::Mean, float, grid);
	filter_typed(image, astro::image::filter::Mean, double, grid);
	return 0;
}

#define	filter_typed1(image, f, pixel, grid)				\
	{								\
		Image<pixel>	*imagep					\
			= dynamic_cast<Image<pixel> *>(&*image);	\
		if (NULL != imagep) {					\
			f<pixel, double>	m;			\
			return m.filter(*imagep, grid);			\
		}							\
	}

double	median(const ImagePtr& image, const Subgrid grid) {
	filter_typed1(image, astro::image::filter::Median, unsigned char, grid);
	filter_typed1(image, astro::image::filter::Median, unsigned short, grid);
	filter_typed1(image, astro::image::filter::Median, unsigned int, grid);
	filter_typed1(image, astro::image::filter::Median, unsigned long, grid);
	filter_typed1(image, astro::image::filter::Median, float, grid);
	filter_typed1(image, astro::image::filter::Median, double, grid);
	return 0;
}

double	max(const ImagePtr& image, const Subgrid grid) {
	filter_typed1(image, astro::image::filter::Max, unsigned char, grid);
	filter_typed1(image, astro::image::filter::Max, unsigned short, grid);
	filter_typed1(image, astro::image::filter::Max, unsigned int, grid);
	filter_typed1(image, astro::image::filter::Max, unsigned long, grid);
	filter_typed1(image, astro::image::filter::Max, float, grid);
	filter_typed1(image, astro::image::filter::Max, double, grid);
	return 0;
}

double	min(const ImagePtr& image, const Subgrid grid) {
	filter_typed1(image, astro::image::filter::Min, unsigned char, grid);
	filter_typed1(image, astro::image::filter::Min, unsigned short, grid);
	filter_typed1(image, astro::image::filter::Min, unsigned int, grid);
	filter_typed1(image, astro::image::filter::Min, unsigned long, grid);
	filter_typed1(image, astro::image::filter::Min, float, grid);
	filter_typed1(image, astro::image::filter::Min, double, grid);
	return 0;
}

} // namespace filter
} // namespace image
} // namespace astro
