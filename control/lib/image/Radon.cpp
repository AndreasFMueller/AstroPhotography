/*
 * Radon.cpp -- classes and structures for radon related transforms
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Radon.h>
#include <AstroDebug.h>
#include <stdexcept>

namespace astro {
namespace image {
namespace radon {

//////////////////////////////////////////////////////////////////////
// segment implementation
//////////////////////////////////////////////////////////////////////
segment::segment(int x, int y, double w) : _x(x), _y(y), _w(w) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create segment (%d,%d) w=%f",
		_x, _y, _w);
	if (_w < 0) {
		throw std::runtime_error("cannot create segment with "
			"negative weight");
	}
}

typedef enum { LEFT, UP } direction_t;

/**
 * \brief find the entry point into a 
 */
static direction_t	exitpoint(int& nx, int& ny, double r,
		double& inx, double& iny) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "process new point %d,%d", nx, ny);
	iny = ny + 0.5;

	// try to compute the incoming point as one from below
	double	ix2 = r * r - iny * iny;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sqrt(ix2) = %f", sqrt(ix2));
	inx = nx - 0.5;
	if (inx < 0) { inx = 0; }
	if (((inx * inx) < ix2) && (ix2 < ((nx + 0.5) * (nx + 0.5)))) {
		// incomping point comes from below		
		inx = sqrt(ix2);
		return UP;
	}

	// try the compute the incoming point as one from the right
	double	iy2 = r * r - inx * inx;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sqrt(iy2) = %f", sqrt(iy2));
	if (((ny - 0.5) * (ny - 0.5) < iy2) && (iy2 < iny * iny)) {
		iny = sqrt(iy2);
		return LEFT;
	}

	// if we get to this point, then 
	throw std::runtime_error("cannot compute entry point");
}



//////////////////////////////////////////////////////////////////////
// circle class implementation
//////////////////////////////////////////////////////////////////////
void	circle::add_segments(const segment& s) {
	_segments->push_back(s);
	if (s.x() > 0) {
		_segments->push_back(segment(-s.x(),  s.y(), s.w()));
	}
	if (s.y() > 0) {
		_segments->push_back(segment( s.x(), -s.y(), s.w()));
	}
	if ((s.x() > 0) && (s.y() > 0)) {
		_segments->push_back(segment(-s.x(), -s.y(), s.w()));
	}
}

/**
 * \brief Build a circle
 *
 * This constructor uses an algorithm similar to the Breesenham algorithm
 * to find the 
 */
circle::circle(double r) {
	// create the _segments
	_segments = segment_ptr(new segments_t());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "building circle of radius %f", r);
	int	x = lround(r), y = 0;
	int	finaly = x;

	// if the only point is the origin, then the segments array
	// has only one point
	if (x == 0) {
		_segments->push_back(segment(0, 0, 1));
		return;
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "initial point (%d,%d)", x, y);

	// compute the first point, and the coordinates of the second point
	double	entryx = x - 0.5, entryy = -0.5;
	double	exitx = x - 0.5, exity = 0.5;
	direction_t	direction = UP;
	if (hypot(x - 0.5, 0.5) < r) {
		direction = UP;
		exitx = entryx = sqrt(r * r - 0.25);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "vertical segment: x = %f",
			exitx);
	} else {
		direction = LEFT;
		exity = sqrt(r * r - (x - 0.5) * (x - 0.5));
		entryy = -exity;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "x = %f, y = %f", entryx, entryy);
	
	double	w = 2 * (-entryy);
	add_segments(segment(x, y, w));
	//add_segments(segment(y, x, w));

	// report on work to do
	debug(LOG_DEBUG, DEBUG_LOG, 0, "finaly = %d", finaly);

	// now compute entry points until you reach the vertical axis
	while ((y != finaly) || (x > 0)) {
		// go to the next point
		switch (direction) {
		case LEFT:
			debug(LOG_DEBUG, DEBUG_LOG, 0, "direction = LEFT");
			x--;
			break;
		case UP:
			debug(LOG_DEBUG, DEBUG_LOG, 0, "direction = UP");
			y++;
			break;
		}
		entryx = exitx;
		entryy = exity;

		// compute the next point
		direction = exitpoint(x, y, r, exitx, exity);

		// compute the segment length
		double	w = hypot(entryx - exitx, entryy - exity);

		// add a new segment
		add_segments(segment(x, y, w));

		// exit point protocol
		debug(LOG_DEBUG, DEBUG_LOG, 0, "exit: (%.3f,%.3f), r = %f",
			exitx, exity, hypot(exitx, exity));
	}

	// 
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d segments added", _segments->size());
}

/**
 * \brief evaluate pixels along a circle
 */ 
double	circle::value(const ConstImageAdapter<double>& image, int x, int y)
	const {
	segments_t::const_iterator	i;
	double	sum = 0;
	double	weightsum = 0;
	int	wx = image.getSize().width();
	int	wy = image.getSize().height();
	for (i = _segments->begin(); i != _segments->end(); i++) {
		const segment&	s = *i;
		int	ix = x + s.x();
		int	iy = y + s.y();
		if (ix < 0)
			continue;
		if (iy < 0)
			continue;
		if (ix >= wx)
			continue;
		if (iy >= wy)
			continue;
		weightsum += s.w();
		sum += s.w() * image.pixel(ix, iy);
	}
	if (0 == weightsum) {
		return std::numeric_limits<double>::quiet_NaN();
	}
	return sum / weightsum;
}

double	circle::length() const {
	segments_t::const_iterator	i;
	double	sum = 0;
	for (i = _segments->begin(); i != _segments->end(); i++) {
		sum += i->w();
	}
	return sum;
}

//////////////////////////////////////////////////////////////////////
// CircleAdapter implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief Construct a Circle Adapter
 */
CircleAdapter::CircleAdapter(const ConstImageAdapter<double>& image,
	const circle& circ) : ConstImageAdapter<double>(image.getSize()),
				_circ(circ), _image(image) {
}

/**
 * \brief Destructor for the Circle Adapter
 */
CircleAdapter::~CircleAdapter() {
}

/**
 * \brief compute circular average
 */
double	CircleAdapter::pixel(int x, int y) const {
	return _circ.value(_image, x, y);
}

} // namespace radon
} // namespace image
} // namespace astro
