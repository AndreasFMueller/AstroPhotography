/*
 * MinRadius.cpp -- minimum radius function
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroFilter.h>
#include <Miniball.hpp>
#include <AstroDebug.h>

namespace astro {
namespace image {
namespace filter {

/**
 *  \brief Function to compute the radius of a ball contining a list of points
 */
double	MinRadius(const std::list<ImagePoint>& points, Point& cp) {
	if (points.size() < 2) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "not enough points: %d",
			points.size());
		return 0.;
	}

	// convert the list of points into a list of vectors, which is
	// more like what the Miniball class expects
	std::list<std::vector<double> >	dpoints;
	std::list<ImagePoint>::const_iterator	i;
	for (i = points.begin(); i != points.end(); i++) {
		std::vector<double>	p;
		p.push_back(i->x());
		p.push_back(i->y());
		dpoints.push_back(p);
	}

	// define the accessor iterators
	typedef std::list<std::vector<double> >::const_iterator	PointIterator;
	typedef std::vector<double>::const_iterator	CoordIterator;

	typedef Miniball::Miniball<Miniball::CoordAccessor<PointIterator, CoordIterator> >	MB;
	
	// now create a miniball problem
	MB	mb(2, dpoints.begin(), dpoints.end());

	// access the results
	const double	*center = mb.center();
	cp = Point(center[0], center[1]);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "center: %s", cp.toString().c_str());
	double	radius = sqrt(mb.squared_radius());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "radius: %f", radius);
	return radius;
}

double	MinRadius(const std::list<ImagePoint>& points, ImagePoint& cp) {
	Point	center;
	double	radius = MinRadius(points, center);
	cp = ImagePoint(cp);
	return radius;
}

double	MinRadius(const std::list<ImagePoint>& points) {
	ImagePoint	where;
	return MinRadius(points, where);
}

} // namespace filter
} // namespace image
} // namespace astro
