/*
 * PeakFinder.cpp -- find the maximum in an image
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroImage.h>
#include <AstroFilter.h>
#include <AstroDebug.h>
#include <AstroFormat.h>

namespace astro {
namespace image {
namespace filter {

/**
 * \brief Criterion to select pixels based on minimum value and distance
 *
 * Points are rejected if the are further away from _approximate than _radius.
 * Pixel values are rejected if the are below _minimum 
 */
class MinimumPixelValueCriterion : public PixelCriterion<double> {
	ImagePoint	_approximate;
	int	_radius;
	double	_minimum;
public:
	MinimumPixelValueCriterion(const ImagePoint approximate, int radius,
		double minimum)
		: _approximate(approximate), _radius(radius), _minimum(minimum)
		{ }
	virtual bool	operator()(const ImagePoint& p, const double& v) {
		if (v < _minimum) return false;
		if (_approximate.distance(p) > _radius) return false;
		return true;
	}
};

/**
 * \brief Perform some plausibility checks on peak finder parameters
 */
void	PeakFinder::setup() {
	// make sure the radius is not too small
	if (_radius < 5) {
		std::string	cause
			= stringprintf("peak finder radius %d too small",
				_radius);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw std::range_error(cause);
	}
	// summarize the parameters
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"peak finder looking for maximum near %s, "
		"radius = %d, maximum = %f",
		_approximate.toString().c_str(), _radius, _maximum);
}

/**
 * \brief Find a peak within a given radius
 *
 * \param radius	the radius to search for the peak
 * \param maximum	The peak value may not exceed this value
 */
PeakFinder::PeakFinder(int radius, double maximum)
	: _radius(radius), _maximum(maximum) {
	setup();
}

/**
 * \brief Create a new PeakFinder object
 *
 * The radius must be at least 2, smaller radii don't make sense
 *
 * \param approximate		the approximate point
 * \param radius		the radius to search for for a peak
 */
PeakFinder::PeakFinder(const ImagePoint& approximate, int radius,
	double maximum)
	: _approximate(approximate), _radius(radius), _maximum(maximum) {
	setup();
}

/**
 * \brief Determine the connected component of pixels above value v
 *
 * 
 *
 * \param image		the image to inspect
 * \param v		the minimum value
 */
WindowedImage<unsigned char>	*PeakFinder::above(
		const ConstImageAdapter<double>& image,
		const ImagePoint& candidate, double v) {
	MinimumPixelValueCriterion	criterion(candidate, _radius, v);
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"connected component for value %f, point %s has value %f",
		v, candidate.toString().c_str(), image.pixel(candidate));

	// initialize an image for the connected component
	ImageRectangle	rectangle = roi(image, candidate);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got the roi rectangle: %s",
		rectangle.toString().c_str());
	ConnectedComponent<double>	cp(candidate, rectangle, criterion);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start getting the CC");
	return cp(image);
}

/**
 * \brief Find a threshold that gives a reasonable number of pixels
 *
 * To compute the centroid, we have to include a reasonable number of
 * pixels around the maximum, between 25 and 49. This method uses interval
 * division to determine such a value.
 *
 * \param image		the image to inspect
 * \param minvalue	minimum value of pixels
 * \param maxvalue	maximum value of pixels
 * \param count		number of pixels that can be in the connected component
 */
std::pair<WindowedImage<unsigned char>*,double>	PeakFinder::threshold(
				const ConstImageAdapter<double>& image,
				const ImagePoint& candidate,
				double minvalue, double maxvalue,
				int suggested) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"thresholding in [%f,%f], image%s=%f, suggested=%d",
		minvalue, maxvalue, candidate.toString().c_str(),
		image.pixel(candidate), suggested);
	// check the suggested number of pixels (should be at least 3)
	if (suggested < 3) {
		suggested = 3;
	}

	// now start looking for a value so that about 3/8 of the pixels
	// have a value between minvalue and maxvalue
	int maxpixelcount = suggested;
//	if (maxpixelcount > 100) {
//		maxpixelcount = 100;
//	}
	const int minpixelcount = maxpixelcount / 2;
	const int targetcount = (minpixelcount + maxpixelcount) / 2;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "targetting %d pixels between %d and %d",
		targetcount, minpixelcount, maxpixelcount);

	// component bitmask
	WindowedImage<unsigned char>	*cc = NULL;

	// now search for a value of the lower limit so that 
	double	vlow = minvalue;
	double	vhigh = maxvalue;
	int	nlow = M_PI * _radius * _radius / 2;
	int	nhigh = 1;
	int	iterations = 32;
	int	pixels = 0;
	double	v = 0;
	do {
		// subdivide the interval
		v = (vlow + vhigh) / 2;
		// count the number of pixels above this level
		if (cc) { delete cc; }
		cc = above(image, candidate, v);
		pixels = ConnectedComponentBase::count(*cc);
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"connected component has %d pixels", pixels);
		if ((pixels > minpixelcount) && (pixels < maxpixelcount)) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"FINAL threshold %g gives %d pixels (%d iterations)",
				v, pixels, 32 - iterations);
			return std::make_pair(cc, v);
		}
		double	vnew = (vlow + vhigh) / 2;
		if (pixels < targetcount) {
			vhigh = vnew;
			nhigh = pixels;
		}
		if (pixels > targetcount) {
			vlow = vnew;
			nlow = pixels;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "values %f:%f pixelcount: %d:%d",
			vlow, vhigh, nlow, nhigh);
	} while (iterations--);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found target level %f", v);

	// if we get to this point, then we did not find an optimal level,
	// so we have to be content with a less optimal choice. If the number
	// of pixels is positive and less than half the area of the image,
	// we run with it anyway
	if ((pixels > 0) && (pixels < image.getSize().getPixels() / 3)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"working with suboptimal level %.3f", v);
		return std::make_pair(cc, v);
	}

	// now if even that does not work, give up
	std::string	cause = stringprintf("no suitable level found");
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
	if (cc) { delete cc; }
	throw std::runtime_error(cause);
}

/**
 * \brief Compute the centroid of the values above the threshold
 *
 * \param image		the image to analize
 * \param threshold	the threshold value above which a pixel is included
 */
std::pair<Point, double>	PeakFinder::centroid(
				const ConstImageAdapter<double>& image,
				const ImagePoint& candidate,
				ConstImageAdapter<unsigned char> *component) {
	// average pixels in the connected component
	int	counter = 0;
	double	xsum = 0, ysum = 0, totalweight = 0;
	ImageRectangle	rectangle = roi(image, candidate);
	int	xmin = rectangle.xmin();
	int	ymin = rectangle.ymin();
	int	xmax = rectangle.xmax();
	int	ymax = rectangle.ymax();
	for (int x = xmin; x < xmax; x++) {
		for (int y = ymin; y < ymax; y++) {
			double	d = hypot(x - candidate.x(),
					y - candidate.y());
			if (d > _radius) {
				// skip points outside the radius
				continue;
			}
			if (component->pixel(x, y) == 255) {
				double	v = image.pixel(x, y);
				xsum += v * x;
				ysum += v * y;
				totalweight += v;
				counter++;
			}
		}
	}
	Point	result(xsum / totalweight, ysum / totalweight);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d pixels averaged to %s", counter,
		result.toString().c_str());

	// return the result
	return std::make_pair(result, totalweight / counter);
}

/**
 * \brief find the maximum in an image
 *
 * \param image		the image to find the peak
 */
Point	PeakFinder::operator()(const ConstImageAdapter<double>& image) {
	return peak(image).first;
}

/**
 * \brief Check whether the candidate is within
 *
 * \param candidate
 */
void	PeakFinder::checkBoundary(const ConstImageAdapter<double>& image,
		const ImagePoint& candidate) const {
	// get the image rectangle
	ImageRectangle	rectangle(image.getSize());

	// make sure the candidate is within the image
	int	borderdistance = rectangle.borderDistance(candidate);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "candidate %s has border distance %d",
		candidate.toString().c_str(), borderdistance);
	if (borderdistance < _radius) {
		std::string	msg = stringprintf("candidate at %s is "
			"not well (%d < %d pixels) inside %s",
			candidate.toString().c_str(), borderdistance,
			_radius, rectangle.toString().c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s within boundaries %s",
		candidate.toString().c_str(),
		rectangle.toString().c_str());
}

/**
 * \brief Find the rectangle for the region if interest
 *
 * \param image		the image, just used for the size
 * \param center	the center of the region of interest
 */
ImageRectangle	PeakFinder::roi(const ConstImageAdapter<double>& image,
		const ImagePoint& center) const {
	int	xmin = center.x() - _radius;
	if (xmin < 0) {
		xmin = 0;
	}
	int	ymin = center.y() - _radius;
	if (ymin < 0) {
		ymin = 0;
	}
	int	xmax = center.x() + _radius;
	if (xmax > image.getSize().width()) {
		xmax = image.getSize().width();
	}
	int	ymax = center.y() + _radius;
	if (ymax > image.getSize().height()) {
		ymax = image.getSize().height();
	}
	int	xlen = xmax - xmin;
	int	ylen = ymax - ymin;
	return ImageRectangle(ImagePoint(xmin, ymin), ImageSize(xlen, ylen));
}

/**
 * \brief 
 *
 * \param image		the image to search for a peak
 */
std::pair<ImagePoint,double>	PeakFinder::globalcandidate(
			const ConstImageAdapter<double>& image) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for global maximum in %s",
		image.getSize().toString().c_str());
	// first get the pixel with the largest value
	Max<double, double>	maxfilter;
	double	maxvalue = maxfilter(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "maximum value: %f", maxvalue);

	// make sure the max value is smaller than the allowed maximum
	if ((_maximum > 0.) && (maxvalue > _maximum)) {
		std::string	msg = stringprintf("maximum too large: %f > %f",
			maxvalue, _maximum);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// get the maximum point
	ImagePoint	candidate = maxfilter.getPoint();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got candidate %s",
		candidate.toString().c_str());

	// make sure the candidate is within the image
	checkBoundary(image, candidate);

	// accept the candidate
	return std::make_pair(candidate, maxvalue);
}

/**
 * \brief
 *
 * \param image		the image to search for the peak
 */
std::pair<ImagePoint,double>	PeakFinder::closecandidate(
			const ConstImageAdapter<double>& image,
			const ImagePoint& closepoint) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for candidate close to %s",
		closepoint.toString().c_str());
	// compute the rectangle to scan
	ImageRectangle	rectangle = roi(image, closepoint);
	int	xmin = rectangle.xmin();
	int	ymin = rectangle.ymin();
	int	xmax = rectangle.xmax();
	int	ymax = rectangle.ymax();

	// start looking for the maximum
	double	value = 0.;
	ImagePoint	candidate;
	for (int x = xmin; x < xmax; x++) {
		for (int y = ymin; y < ymax; y++) {
			double	d = hypot(x - closepoint.x(),
					y - closepoint.y());
			if (d > _radius)
				continue;
			double newvalue = image.pixel(x, y);
			if (newvalue > value) {
				candidate = ImagePoint(x, y);
				value = newvalue;
			}
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got candidate %s",
		candidate.toString().c_str());

	// make sure the candidate is within the image
	checkBoundary(image, candidate);

	// accept the candidate
	return std::make_pair(candidate, value);
}

/**
 * \brief Find the peak and its weight
 *
 * \param image		the image to analyze
 */
std::pair<Point, double>	PeakFinder::peak(
			const ConstImageAdapter<double>& image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for peak in %s image",
		image.getSize().toString().c_str());
	// try to 
	double	circlearea = M_PI * _radius *_radius;
	unsigned int	suggested = circlearea / 2;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "half circle area: %u", suggested);

	// we have to make sure the candidate peak is not too close
	// to the boundary of the image. To this end, we require that 
	// it is at least _radius/4 away from the boundary
	ImagePoint	candidate;
	double	maxvalue;

	// if the approximate centroid was not set, set it from the
	// maximum filter
	std::pair<ImagePoint, double>	candmax;
	if (_approximate == ImagePoint()) {
		candmax = globalcandidate(image);
	} else {
		candmax = closecandidate(image, _approximate);
	}
	candidate = candmax.first;
	maxvalue = candmax.second;
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"find peak near candidate %s, value %f/%f",
		candidate.toString().c_str(), image.pixel(candidate), maxvalue);

	// we now search within a radius of _radius around the
	// point with the maximum value
	double	minvalue = maxvalue;
	ImageRectangle	rectangle = roi(image, candidate);
	int	xmin = rectangle.xmin();
	int	ymin = rectangle.ymin();
	int	xmax = rectangle.xmax();
	int	ymax = rectangle.ymax();
	int	counter = 0;
	double	sum = 0;
	for (int x = xmin; x < xmax; x++) {
		for (int y = ymin; y < ymax; y++) {
			double	d = hypot(x - candidate.x(),
					y - candidate.y());
			if (d > _radius) {
				continue;
			}
			counter++;
			double	v = image.pixel(x, y);
			sum += v;
			if (v < minvalue) {
				minvalue = v;
			}
		}
	}
	double	mean = sum/counter;
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"radius circle contains %d pixels between %f and %f, mean = %f",
		counter, minvalue, maxvalue, mean);

	// count the number of points in the connected component between
	// mean and maxvalue
	debug(LOG_DEBUG, DEBUG_LOG, 0, "counting interesting pixels above %f",
		mean);
	MinimumPixelValueCriterion	criterion(candidate, _radius, mean);
	ConnectedComponent<double>	cp(candidate, rectangle, criterion);
	WindowedImage<unsigned char>	*meancomp = cp(image);
	unsigned int	interesting
		= ConnectedComponentBase::count(*meancomp, rectangle);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %u interesting pixels",
		interesting);
	delete meancomp;

	// derive a suggestion on how many pixels should be target
	// while thresholding
	if (interesting < suggested) {
		suggested = interesting;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "suggested number of pixels %u",
		suggested);
	
	// now find a threshold level that gives a reasonable number
	// of contributing pixels
	std::pair<WindowedImage<unsigned char>*,double>	th
		= threshold(image, candidate, minvalue, maxvalue, suggested);

	// compute the centroid around this point
	std::pair<Point, double>	c = centroid(image, candidate,
		th.first);
	delete th.first;

	// done
	Point	result = c.first;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found peak: %s, weight = %f",
		result.toString().c_str(), c.second);
	return std::make_pair(result, c.second);
}

} // namespace filter
} // namespace image
} // namespace astro
