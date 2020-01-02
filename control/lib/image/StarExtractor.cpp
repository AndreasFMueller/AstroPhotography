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
 * \brief Constructor for the StarExtractor class
 */
StarExtractor::StarExtractor(int numberofstars, int searchradius)
	: _numberofstars(numberofstars), _searchradius(searchradius), 
	  _saturation(1.0) {
}

StarExtractor::StarExtractor(const StarExtractor& other)
	: _numberofstars(other._numberofstars),
	  _searchradius(other._searchradius), 
	  _saturation(other._saturation) {
}


/**
 * \brief Main star extractor method
 *
 * This method looks for large values in an image and determines their
 * properties of a star.
 */
std::vector<Star>	StarExtractor::stars(
			const ConstImageAdapter<double>& image,
			const StarAcceptanceCriterion& criterion) const {
	std::vector<Star>	result;
	// find the maximum value in the image
	double	m = filter::Max<double, double>().filter(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "maximum value: %f", m);

	// create a level extractor that looks for stars with a brightness
	// at a certain level
	LevelExtractor	extractor(m);
	do {
		// while we don't have enough stars, lower the level at which
		// we are looking for stars
		extractor.level(extractor.level() / 2);
		extractor.analyze(image, criterion);
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
				const ConstImageAdapter<double>& image,
				const StarAcceptanceCriterion& criterion) const {
	return stars2points(stars(image, criterion));
}

#define do_extract(image, criterion, Pixel)				\
	{								\
		Image<Pixel >	*imagep					\
			= dynamic_cast<Image<Pixel >*>(&*image);	\
		if (NULL != imagep) {					\
			TypedStarExtractor<Pixel>	tsa(*this);	\
			return tsa.stars(*imagep, criterion);		\
		}							\
	}

/**
 * \brief Extract a set of stars from an ImagePtr
 */
std::vector<Star>	StarExtractor::stars(ImagePtr image,
				const StarAcceptanceCriterion& criterion) const {
	do_extract(image, criterion, unsigned char)
	do_extract(image, criterion, unsigned short)
	do_extract(image, criterion, unsigned int)
	do_extract(image, criterion, unsigned long)
	do_extract(image, criterion, float)
	do_extract(image, criterion, double)
	do_extract(image, criterion, RGB<unsigned char>)
	do_extract(image, criterion, RGB<unsigned short>)
	do_extract(image, criterion, RGB<unsigned int>)
	do_extract(image, criterion, RGB<unsigned long>)
	do_extract(image, criterion, RGB<float>)
	do_extract(image, criterion, RGB<double>)
	std::string	msg = stringprintf("cannot find stars in image %s "
		"pixels", demangle(image->pixel_type().name()).c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief Extract a set of points from an ImagePtr
 */
std::vector<Point>	StarExtractor::points(ImagePtr image,
		const StarAcceptanceCriterion& criterion) const {
	return stars2points(stars(image, criterion));
}

} // namespace transform
} // namespace image
} // namespace astro
