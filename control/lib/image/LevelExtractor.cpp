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
}

/**
 * \brief Accessor for the n brightest stars
 */
std::vector<Star>	LevelExtractor::stars(int n) {
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
 * \brief inspect a point of an image
 */
int	LevelExtractor::inspectpoint(const ConstImageAdapter<double>& image,
		int x, int y) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "inspect point %d,%d", x, y);
	// make sure this point is far enough from all previous points
	for (auto ptr = _stars.begin(); ptr != _stars.end(); ptr++) {
		double	d = distance(*ptr, Point(x, y));
		if (d < _radius) {
			int	skip = 2 * fabs(y - ptr->y());
			if (skip < 1) {
				skip = 1;
			}
			debug(LOG_DEBUG, DEBUG_LOG, 0, "skip = %d", skip);
			return skip;
		}
	}

	// so we have a possible candidate at (x, y). To make this into a
	// star, we have to look for a peak within the search radius around
	// this point
	int	r = _radius;
	ImageRectangle	rectangle(ImagePoint(x - r, y - r),
				ImageSize(2 * r, 2 * r));
	adapter::WindowAdapter<double>	window(image, rectangle);
	filter::PeakFinder	pf(ImagePoint(x, y), _radius);
	try {
		std::pair<Point,double>	p = pf.peak(window);
		//Star	star(p.first + Point(rectangle.origin()), p.second);
		Star	star(p.first, p.second);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found star: %s",
			star.toString().c_str());
		_stars.insert(_stars.begin(), star);

		// number of points we can skip after adding a star
		int	skip = _radius;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "skipping new star: %d", skip);
		return skip;
	} catch (const std::exception& x) {
	}
	return 1;
}

/**
 * \brief inspect an image
 */
void	LevelExtractor::analyze(const ConstImageAdapter<double>& image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "analyzing %s image, at level %f",
		image.getSize().toString().c_str(), _level);
	// clear the current star set
	_stars.clear();

	// create an adapter
	double	m = filter::Mean<double, double>().filter(image);
	if (m > _level) {
		m = _level / 2;
	}
	double	_limit = 1.01 * _level - m;
	ReductionAdapter	reducedimage(image, m, 1.01 * _level);

	// get the image dimensions
	int	w = image.getSize().width();
	int	h = image.getSize().height();
	int	r = _radius;

	// scan for high values
	for (int x = r; x < w - r; x++) {
		int	y = r;
		while (y < h - r) {
			double	v = reducedimage.pixel(x, y);
			if (v < _limit) {
				y++;
				continue;
			}
			int	skip = inspectpoint(reducedimage, x, y);
			y += skip;
		}
	}
}

} // namespace transform
} // namespace image
} // namespace astro
