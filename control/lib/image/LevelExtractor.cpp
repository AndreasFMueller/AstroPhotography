/*
 * LevelExtractor.cpp -- class to extract stars from an image
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "LevelExtractor.h"
#include <limits>
#include "ReductionAdapter.h"

namespace astro {
namespace image {
namespace transform {

/**
 * \brief Constructor for the level extractor
 */
LevelExtractor::LevelExtractor(double level) : _level(level) {
	_radius = 16;
	_maxstars = 20;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "LevelExtractor at level %f", _level);
}

/**
 * \brief Accessor for the n brightest stars
 *
 * \param n	number of stars to extract
 */
std::vector<Star>	LevelExtractor::stars(unsigned int n) {
	if (n > _stars.size()) {
		std::string	msg = stringprintf("have not enough stars: "
			"%d < %d", _stars.size(), n);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::range_error(msg);
	}
	std::vector<Star>	result;
	auto	ptr = _stars.rbegin();
	while (n > 0) {
		result.push_back(*ptr++);
		n--;
	}
	return result;
}

/**
 * \brief Check whether a point is close to an existing star
 *
 * \param x	the x coordinate of the new star
 * \param y	the y coordinate of the new star
 * \return	the number of pixels to skip to get away from all existing
 *		stars
 */
int	LevelExtractor::close(int x, int y) const {
	int	 skip = -1;
	// make sure this point is far enough from all previous points
	for (auto ptr = _stars.begin(); ptr != _stars.end(); ptr++) {
		double	d = distance(*ptr, Point(x, y));
		if (d < _radius) {
			skip = 2 * fabs(y - ptr->y());
			if (skip < 1) {
				skip = 1;
			}
			debug(LOG_DEBUG, DEBUG_LOG, 0, "close star: skip = %d",
				skip);
			return skip;
		}
	}
	// if we get to this point, the point (x,y) is not close to any
	// existing star
	return -1;
}

/**
 * \brief inspect a point of an image
 *
 * \return	number of pixels to skip after this point 
 */
int	LevelExtractor::inspectpoint(const ConstImageAdapter<double>& image,
		int x, int y, double limit,
		const StarAcceptanceCriterion& criterion) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "inspect point %d,%d", x, y);
	// make sure this point is far enough from all previous points
	int	skip = close(x, y);
	if (skip > 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "close star: skip = %d", skip);
		return skip;
	}

	// so we have a possible candidate at (x, y). To make this into a
	// star, we have to look for a peak within the search radius around
	// this point
	int	r = _radius;
	ImageRectangle	rectangle(ImagePoint(x - r, y - r),
				ImageSize(2 * r, 2 * r));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "inspecting rectangle %s",
		rectangle.toString().c_str());
	adapter::WindowAdapter<double>	window(image, rectangle);
	//filter::PeakFinder	pf(ImagePoint(x, y), _radius/2, limit);
	filter::PeakFinder	pf(_radius/2, limit);
	try {
		std::pair<Point,double>	p = pf.peak(window);
		Point	center = p.first + Point(rectangle.origin());
		ImagePoint	ip(center.x(), center.y());
		Star	star(center, image.pixel(ip));
		//Star	star(p.first, p.second);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found star: %s",
			star.toString().c_str());

		// check whether the star is acceptable
		if (criterion.accept(star)) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "star %s accepted",
				star.toString().c_str());
			_stars.insert(_stars.begin(), star);
			// compute the y values to skip
			skip = close(x, y);
			if (skip < 0) {
				skip = 1;
			}
		} else {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "star %s rejected",
				star.toString().c_str());
			// even when we rejected a star, we still keep away
			// from this maximum
			skip = _radius;
		}

		// number of points we can skip after adding a star
		debug(LOG_DEBUG, DEBUG_LOG, 0, "skipping new star: %d", skip);
		return skip;
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no peak found: %s", x.what());
		// we skip a _radius, because if there was a peak close by
		// we would certainly see it if we jump ahead by _radius
		return skip = _radius / 2;
	}
	return 1;
}

/**
 * \brief inspect an image
 *
 * \param image		the image to analyze
 * \param criterion	the criterion for accepting a star
 */
void	LevelExtractor::analyze(const ConstImageAdapter<double>& image,
		const StarAcceptanceCriterion& criterion) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "analyzing %s image, at level %f",
		image.getSize().toString().c_str(), _level);
	// clear the current star set
	_stars.clear();

	// we are looking for peaks in brightness between the mean
	// and the _level.
	double	m = filter::Mean<double, double>().filter(image);
	if (m > _level) {
		// if the _level is smaller than the mean, take
		// half the _level as the lower limit
		m = _level / 2;
	}
	// the limit is the value the reduced image will have at most
	double	_limit = _level - m;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "min = %f, max = %f, limit = %f",
		m, _level, _limit);

	// create an adapter that returns value between 0 and 1.01*_level-m
	// (or _limit).
	ReductionAdapter	reducedimage(image, m, 1.01 * _level);

	// get the image dimensions
	int	w = image.getSize().width();
	int	h = image.getSize().height();
	int	r = _radius;
	int	step = r / 4;
	if (step == 0) {
		step = 1;
	}

	// scan for high values
	for (int x = r; x < w -r; x += step) {
		for (int y = r; y < h - r; y += step) {
#if 0
			double	v = reducedimage.pixel(x, y);
			// skip all points that have a value > _limit, they
			// belong to stars at a higher level
			if (v > _limit) {
				y += r;
				continue;
			}
			// skip 0 values
			if (v == 0.) {
				y++;
				continue;
			}
#endif
			// at this point we are looking at a pixel value
			// between m and _level
			int	skip = inspectpoint(reducedimage, x, y, _limit,
					criterion);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "skip = %d", skip);
		}
	}
}

} // namespace transform
} // namespace image
} // namespace astro
