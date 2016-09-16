/*
 * StarExtractor.cpp -- class to extract stars from an image
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTransform.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroFilter.h>
#include "LevelExtractor.h"

namespace astro {
namespace image {
namespace transform {

/**
 * \brief Main star extractor method
 *
 * This method looks for large values in an image and determines their
 * properties of a star.
 */
std::vector<Star>	StarExtractor::stars(
				const ConstImageAdapter<double>& image) const {
	std::vector<Star>	result;
	// find the maximum value in the image
	double	m = filter::Max<double, double>().filter(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "maximum value: %f", m);

	// create a level extractor
	LevelExtractor	extractor(m);
	do {
		extractor.level(extractor.level() / 2);
		extractor.analyze(image);
	} while (_numberofstars > extractor.nstars());
	return extractor.stars(_numberofstars);
}

/**
 * \brief auxiliary function to convert a set of of stars into a set of points
 */
std::vector<Point> StarExtractor::stars2points(const std::vector<Star>& stars) {
	std::vector<Point>	result;
	for (auto ptr = stars.begin(); ptr != stars.end(); ptr++) {
		Point	p = *ptr;
		result.push_back(p);
	}
	return result;
}

/**
 * \brief Extract a set of star points from an image
 */
std::vector<Point>	StarExtractor::points(
				const ConstImageAdapter<double>& image) const {
	return stars2points(stars(image));
}

#define do_extract(image, Pixel)					\
	{								\
		Image<Pixel >	*imagep					\
			= dynamic_cast<Image<Pixel >*>(&*image);	\
		if (NULL != imagep) {					\
			TypedStarExtractor<Pixel>	tsa(*this);	\
			return tsa.stars(*imagep);			\
		}							\
	}

/**
 * \brief Extract a set of stars from an ImagePtr
 */
std::vector<Star>	StarExtractor::stars(ImagePtr image) const {
	do_extract(image, unsigned char)
	do_extract(image, unsigned short)
	do_extract(image, unsigned int)
	do_extract(image, unsigned long)
	do_extract(image, float)
	do_extract(image, double)
	do_extract(image, RGB<unsigned char>)
	do_extract(image, RGB<unsigned short>)
	do_extract(image, RGB<unsigned int>)
	do_extract(image, RGB<unsigned long>)
	do_extract(image, RGB<float>)
	do_extract(image, RGB<double>)
	std::string	msg = stringprintf("cannot find stars in image %s "
		"pixels", demangle(image->pixel_type().name()).c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief Extract a set of points from an ImagePtr
 */
std::vector<Point>	StarExtractor::points(ImagePtr image) const {
	return stars2points(stars(image));
}

} // namespace transform
} // namespace image
} // namespace astro
