/*
 * Radon.cpp -- Radon transform implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, HOchschule Rapperswil
 */
#include <Radon.h>
#include <AstroDebug.h>
#include <limits>
#include <AstroFormat.h>
#include <AstroTypes.h>

namespace astro {
namespace image {
namespace radon {

/**
 * \brief Auxiliary class to handle normal vectors for the lines
 *
 * The Radon transform computes integrals along lines in an image, 
 * this class implements all the computations needed for this integration
 */
class Normal {
	double	_nx;
	double	_ny;
public:
	typedef enum { DIRECTIONX, DIRECTIONY } direction_type;
private:
	direction_type	_direction;
public:
	direction_type	direction() const { return _direction; }
	typedef enum { RIGHT, UP, LEFT, DOWN } walk_direction;
private:
	walk_direction	_walk;
public:
	walk_direction	walk() const { return _walk; }
	Normal(double angle) {
		_nx = cos(angle);
		_ny = sin(angle);
		if (fabs(_nx) < fabs(_ny)) {
			_direction = DIRECTIONX;
		} else {
			_direction = DIRECTIONY;
		}
	}
	std::string	toString() const {
		return stringprintf("(%.4f,%.4f)", _nx, _ny);
	}
	double	tan() const { return _ny / _nx; }
	double	cot() const { return _nx / _ny; }
	double	csc() const { return 1. / _ny; }
	double	sec() const { return 1. / _nx; }
	double	scalar(int x, int y) const {
		return _nx * x + _ny * y;
	}
	double	scalar(double x, double y) const {
		return _nx * x + _ny * y;
	}
	double	scalar(const ImageSize& size) const {
		return scalar(size.width(), size.height());
	}
	double	scalar(const ImagePoint& point) const {
		return scalar(point.x(), point.y());
	}
	double	scalar(const Point& point) const {
		return scalar(point.x(), point.y());
	}
	bool	xdirection() const {
		return fabs(_nx) <= fabs(_ny);
	}
	bool	ydirection() const {
		return fabs(_nx) > fabs(_ny);
	}
private:
	bool	samesign(double a, double b) const {
		if (((a > 0) && (b > 0)) || ((a < 0) && (b < 0))) {
			return true;
		}
		return false;
	}
	int	closest(double s, double increment) const {
		int	direction = (samesign(s, increment)) ? -1 : 1;
		double	alternative = s + increment * direction;
		if (fabs(alternative) < fabs(s)) {
			return direction;
		}
		return 0;
	}
public:
	ImagePoint	nextPoint(const ImagePoint& point,
				double& deltas) const {
		int	d;
		switch (_direction) {
		case DIRECTIONX:
			d = closest(deltas + _nx, _ny);
			deltas += scalar(1, d);
			return ImagePoint(point.x() + 1, point.y() + d);
		case DIRECTIONY:
			d = closest(deltas + _ny, _nx);
			deltas += scalar(d, 1);
			return ImagePoint(point.x() + d, point.y() + 1);
		}
	}
	typedef std::pair<ImagePoint, ImagePoint>	pointpair;
	pointpair	endpoints(double s, const ImageSize& size) const;
	pointpair	roundpoints(const Point& p1, const Point& p2) const;
};

Normal::pointpair	Normal::roundpoints(const Point& p1,
				const Point& p2) const {
	ImagePoint	point1(p1.x(), p1.y());
	ImagePoint	point2(p2.x(), p2.y());
	switch (_direction) {
	case DIRECTIONX:
		if (point1.x() < point2.x()) {
			return pointpair(point1, point2);
		} else {
			return pointpair(point2, point1);
		}
		break;
	case DIRECTIONY:
		if (point1.y() < point2.y()) {
			return pointpair(point1, point2);
		} else {
			return pointpair(point2, point1);
		}
		break;
	}
}

/**
 * \brief Compute the start and end points of a line through the image
 *
 * This method computes the start and end points of a line that 
 * crosses the borders of the two 
 */
Normal::pointpair	Normal::endpoints(double s, const ImageSize& size) const {
	Size	r(size.width(), size.height());
	// find points on edges
	double	lefty = std::numeric_limits<double>::infinity();
	double	righty = std::numeric_limits<double>::infinity();
	if (_ny != 0) {
		lefty = s / _ny;
		righty = (s - _nx * (size.width() - 1)) / _ny;
	}
	Point	left(0, lefty);
	Point	right(r.width() - 1, righty);
	double	topx = std::numeric_limits<double>::infinity();
	double	bottomx = std::numeric_limits<int>::infinity();
	if (_nx != 0) {
		topx = (s - _ny * (r.height() - 1)) / _nx;
		bottomx = s / _nx;
	}
	Point	bottom(bottomx, 0);
	Point	top(topx, r.height() - 1);

/*
debug(LOG_DEBUG, DEBUG_LOG, 0, "Normal left = %s", left.toString().c_str());
debug(LOG_DEBUG, DEBUG_LOG, 0, "Normal right = %s", right.toString().c_str());
debug(LOG_DEBUG, DEBUG_LOG, 0, "Normal top = %s", top.toString().c_str());
debug(LOG_DEBUG, DEBUG_LOG, 0, "Normal bottom = %s", bottom.toString().c_str());
 */
	
	// now return the pairs in such a way that we can always step up
	// one coordinate
	switch (_direction) {
	case DIRECTIONX:
// debug(LOG_DEBUG, DEBUG_LOG, 0, "Normal X");
		if (r.contains(left)) {
			if (r.contains(top)) {
				return roundpoints(left, top);
			}
			if (r.contains(bottom)) {
				return roundpoints(left, bottom);
			}
			if (r.contains(right)) {
				return roundpoints(left, right);
			}
		}
		if (r.contains(bottom)) {
			if (r.contains(top)) {
				return roundpoints(bottom, top);
			}
			if (r.contains(right)) {
				return roundpoints(bottom, right);
			}
		}
		if (r.contains(top) && r.contains(right)) {
			return roundpoints(top, right);
		}
		break;
	case DIRECTIONY:
// debug(LOG_DEBUG, DEBUG_LOG, 0, "Normal Y");
		if (r.contains(bottom)) {
			if (r.contains(top)) {
				return roundpoints(bottom, top);
			}
			if (r.contains(left)) {
				return roundpoints(bottom, left);
			}
			if (r.contains(right)) {
				return roundpoints(bottom, right);
			}
		}
		if (r.contains(left)) {
			if (r.contains(top)) {
				return roundpoints(left, top);
			}
			if (r.contains(right)) {
				return roundpoints(left, right);
			}
		}
		if (r.contains(right) && r.contains(top)) {
			return roundpoints(right, top);
		}
		break;
	}
	throw std::runtime_error("no intersection");
}

/**
 * \brief An implementation class for the radon transform
 */
class RadonImplementation {
	/**
	 * \brief Scale of the pixel in the Radon transform
	 *
	 * Each pixel in the radon transform stands for a lane of with
	 * <scale> through the image.
	 */
	double	_scale;
public:
	RadonImplementation(double scale = 1) : _scale(scale) { }
	// Radon transform driver
	void	transform(ImageAdapter<double>& radon,
		const ConstImageAdapter<double>& image) const;
	// iteration along the direction of the normal
	void	iterate(ImageAdapter<double>& radon, 
		const ConstImageAdapter<double>& image, int y, double angle)
		const;
	// integral along a line orthogonal the normal
	double	integral(const ConstImageAdapter<double>& image,
		const Normal& normal, double s) const;
	double	xintegral(const ConstImageAdapter<double>& image,
		const Normal& normal, const ImagePoint& start, double s) const;
	double	yintegral(const ConstImageAdapter<double>& image,
		const Normal& normal, const ImagePoint& start, double s) const;
};

/**
 * \brief Radon transform driver operation
 *
 * This method controls the angle, so it creates vertical lines of the
 * radon transform in each iteration.
 */
void	RadonImplementation::transform(ImageAdapter<double>& radon,
		const ConstImageAdapter<double>& image) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "perform Radon transform on %s image",
		image.getSize().toString().c_str());
	// get the image dimensions
	int	height = radon.getSize().height();
	//int	width = radon.getSize().width();
	double	anglestep = M_PI / height;

	// iterate over the angles
	for (int y = 0; y < height; y++) {
		// now go through the image at this angle 
		double	angle = y * anglestep;
		iterate(radon, image, y, angle);
	}
}

/**
 * \brief Iteration along 
 *
 * This method iterates along the normal direction. It computes the start
 * and end points of the line through the image along which the pixel
 * values should be accumulated.
 */
void	RadonImplementation::iterate(ImageAdapter<double>& radon,
		const ConstImageAdapter<double>& image, int y, double angle)
		const {
	// compute the normal vector
	Normal	normal(angle);

	// s value for the center of the image
	Point	center(image.getSize().center());
	double	scenter = normal.scalar(center - Point(0.5,0.5));
	
	// compute the range along the normal affected by image
	// image pixels
	int	imageWidth = image.getSize().width();
	int	imageHeight = image.getSize().height();
	double	smax = fabs(scenter);
	smax = fmax(smax,
		fabs(normal.scalar(imageWidth, 0) - scenter)
	);
	smax = fmax(smax,
		fabs(normal.scalar(imageWidth, imageHeight) - scenter)
	);
	smax = fmax(smax,
		fabs(normal.scalar(0, imageHeight) - scenter)
	);
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"angle = %.3f, normal = %s, scenter = %.3f, srange = %.2f",
		180 * angle / M_PI, normal.toString().c_str(), scenter, smax);
		
	// compute the range of s values for which we have there is space
	// in the Radon transform image
	int	w = radon.getSize().width();
	int	w2 = w / 2;
#pragma omp parallel for
	for (int si = 0; si < w; si++) {
		// compute the s-value for which we have to compute the
		// integral
		double	s = _scale * (si - w2);
		if ((s < -smax) || (s > smax)) {
			radon.writablepixel(si, y) = 0.;
			continue;
		}

		// handle the case where we are inside the image,
		double	radonvalue = integral(image, normal, s + scenter);
		radon.writablepixel(si, y) = radonvalue;
		//debug(LOG_DEBUG, DEBUG_LOG, 0,
		//	"normal = %s, s = %.3f, value = %f", 
		//	normal.toString().c_str(), s, radonvalue);
	}
}

/**
 * \brief Compute the integral along a line
 *
 * This method computes the integral along a line through the image
 */
double	RadonImplementation::integral(const ConstImageAdapter<double>& image,
		const Normal& normal, double s) const {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "integral with offset s = %f", s);
	// find the start point of the line 
	ImagePoint	start;
	ImagePoint	end;
	try {
		Normal::pointpair	points = normal.endpoints(s,
							image.getSize());
		start = points.first;
		end = points.second;
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "integral from %s to %s",
		//	start.toString().c_str(), end.toString().c_str());
	} catch (const std::exception& x) {
		return 0;
	}
	double	deltas = normal.scalar(start) - s;

	// find out whether the angle of the line is closer to the x-
	// or the y-axis
	if (normal.xdirection()) {
		return xintegral(image, normal, start, deltas);
	} else {
		return yintegral(image, normal, start, deltas);
	}
	return 0;
}

/**
 * \brief Compute the integral along a line closer to the x axis
 */
double	RadonImplementation::xintegral(const ConstImageAdapter<double>& image,
		const Normal& normal, const ImagePoint& start, double deltas)
		const {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "xintegral from %s, deltas = %.3f",
	//	start.toString().c_str(), deltas);
	ImageSize	size = image.getSize();
	double	sum = 0;
	int	counter	= 0;
	ImagePoint	point = start;
	while (size.contains(point)) {
		// add the pixel value at that point
		double	value = image.pixel(point);
		if (value == value) {
			sum += value;
			counter++;
		}

		point = normal.nextPoint(point, deltas);
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "next point: %s, deltas = %.3f",
		//	point.toString().c_str(), deltas);
	}
	double	value = sum * fabs(normal.csc());
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "normal = %s, start = %s, "
	//	"s = %.3f completed, %d points, value = %.1f",
	//	normal.toString().c_str(), start.toString().c_str(),
	//	deltas, counter, value);
	return value;
}

/**
 * \brief Compute the integral along a line closer to the y axis
 */
double	RadonImplementation::yintegral(const ConstImageAdapter<double>& image,
		const Normal& normal, const ImagePoint& start, double deltas)
		const {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "yintegral from %s, deltas = %.3f",
	//	start.toString().c_str(), deltas);
	ImageSize	size = image.getSize();
	double	sum = 0;
	int	counter	= 0;
	ImagePoint	point = start;
	while (size.contains(point)) {
		// add the pixel value at that point
		double	value = image.pixel(point);
		if (value == value) {
			sum += value;
			counter++;
		}

		point = normal.nextPoint(point, deltas);
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "next point: %s, deltas = %.3f",
		//	point.toString().c_str(), deltas);
	}
	double	value = sum * fabs(normal.sec());
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "normal = %s, start = %s, "
	//	"s = %.3f completed, %d points, value = %.1f",
	//	normal.toString().c_str(), start.toString().c_str(),
	//	deltas, counter, value);
	return value;

}

/**
 * \brief Construct a radon transform for a given image
 */
RadonTransform::RadonTransform(const ImageSize& size,
	const ConstImageAdapter<double>& image) 
	: ConstImageAdapter<double>(size), _image(image), _radon(size) {
	RadonImplementation	ri;
	ri.transform(_radon, _image);
}

//////////////////////////////////////////////////////////////////////
// implementation of the RadonAdapter class
//////////////////////////////////////////////////////////////////////
RadonAdapter::RadonAdapter(const ImageSize& size,
                const ConstImageAdapter<double>& image)
	: ConstImageAdapter<double>(ImageSize(size.width(), 2 * size.height())),
	  _radon(size, image) {
}

/**
 * \brief access Radon transform pixels
 */
double	RadonAdapter::pixel(int x, int y) const {
	ImageSize	s = getSize();
	int	w = s.width();
	if ((x < 0) || (x >= w)) {
		return 0.;
	}
	int	h = s.height();
	y = y % h;
	h = h / 2;
	if (y >= h) {
		x = w - x;
		y = y - h;
		if (x >= w) {
			return 0.;
		}
	}
	return _radon.pixel(x, y);
}

} // namespace radon
} // namespace image
} // namespace astro
