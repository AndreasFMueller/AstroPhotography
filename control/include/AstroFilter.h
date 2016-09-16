/*
 * AstroFilter.h -- filters to apply to images 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroFilter_h
#define _AstroFilter_h

#include <AstroImage.h>
#include <AstroAdapter.h>
#include <limits>
#include <AstroDebug.h>
#include <list>
#include <AstroFormat.h>
#include <AstroFilter.h>

namespace astro {
namespace image {
namespace filter {

/**
 * \brief Filters that return a single value of the same type as the image.
 *
 * This type of filter cannot be used to compute values from the image
 * that don't fit in the pixel type. An example of such a value would be
 * the mean value. There is a Mean filter derived from this type but in its
 * basic form it computes the integer rounded version.
 */
template<typename T, typename S>
class PixelTypeFilter {
public:
	virtual S	filter(const ConstImageAdapter<T>& image) = 0;
	virtual T	operator()(const ConstImageAdapter<T>& image) = 0;
};

/**
 * \brief Filter to count NaNs
 */
template<typename T, typename S>
class CountNaNs : public PixelTypeFilter<T, S> {
public:
	CountNaNs() { }
	virtual S	filter(const ConstImageAdapter<T>& image);
	virtual T	operator()(const ConstImageAdapter<T>& image);
};

template<typename T, typename S>
S	CountNaNs<T, S>::filter(const ConstImageAdapter<T>& image) {
	S	result = 0;
	ImageSize	size = image.getSize();
	for (int x = 0; x < size.width(); x++) {
		for (int y = 0; y < size.height(); y++) {
			T	v = image.pixel(x, y);
		
			if ((std::numeric_limits<T>::has_quiet_NaN) && (v != v)) {
				result += 1;
			}
		}
	}
	return result;
}

template<typename T, typename S>
T	CountNaNs<T, S>::operator()(const ConstImageAdapter<T>& image) {
	return (T)filter(image);
}

/**
 * \brief Filter that finds the largest value of all pixels
 */
template<typename T, typename S>
class Max : public PixelTypeFilter<T, S> {
	int	maxx, maxy;
public:
	Max() { maxx = 0; maxy = 0; }
	virtual	S	filter(const ConstImageAdapter<T>& image);
	virtual T	operator()(const ConstImageAdapter<T>& image);
	ImagePoint	getPoint() const { return ImagePoint(maxx, maxy); }
};

template<typename T, typename S>
S	Max<T, S>::filter(const ConstImageAdapter<T>& image) {
	return (S)this->operator()(image);
}

template<typename T, typename S>
T	Max<T, S>::operator()(const ConstImageAdapter<T>& image) {
	T	result = 0;
	maxx = 0;
	maxy = 0;
	ImageSize	size = image.getSize();
	for (int x = 0; x < size.width(); x++) {
		for (int y = 0; y < size.height(); y++) {
			T	v = image.pixel(x, y);
			if (v != v) continue; // skip NaNs
			if (v > result) {
				result = image.pixel(x, y);
				maxx = x;
				maxy = y;
			}
		}
	}
	return result;
}

/**
 * \brief Filter that finds the smalles value of all pixels
 */
template<typename T, typename S>
class Min : public PixelTypeFilter<T, S> {
	int	minx, miny;
public:
	Min() { }
	virtual	S	filter(const ConstImageAdapter<T>& image);
	virtual T	operator()(const ConstImageAdapter<T>& image);
	ImagePoint	getPoint() const { return ImagePoint(minx, miny); }
};

template<typename T, typename S>
S	Min<T, S>::filter(const ConstImageAdapter<T>& image) {
	return (S) this->operator()(image);
}

template<typename T, typename S>
T	Min<T, S>::operator()(const ConstImageAdapter<T>& image) {
	T	result = std::numeric_limits<T>::max();
	minx = 0;
	miny = 0;
	ImageSize	size = image.getSize();
	for (int x = 0; x < size.width(); x++) {
		for (int y = 0; y < size.height(); y++) {
			T	v = image.pixel(x, y);
			if ((std::numeric_limits<T>::has_quiet_NaN) && (v != v)) continue; // skip NaNs
			if (v < result) {
				result = v;
				minx = x;
				miny = y;
			}
		}
	}
	return result;
}

/**
 * \brief Filter that finds the mean of an image
 */
template<typename T, typename S>
class Mean : public PixelTypeFilter<T, S> {
	bool	relative;
public:
	Mean(bool _relative = false) : relative(_relative) {
	}
	virtual S	filter(const ConstImageAdapter<T>& image);
	virtual T	operator()(const ConstImageAdapter<T>& image);
};

template<typename T, typename S>
S	Mean<T, S>::filter(const ConstImageAdapter<T>& image) {
	ImageSize	size = image.getSize();
	S	sum = 0;
	size_t	counter = 0;
	bool	check_nan = std::numeric_limits<T>::has_quiet_NaN;
	for (int x = 0; x < size.width(); x++) {
		for (int y = 0; y < size.height(); y++) {
			T	v = image.pixel(x, y);
			if ((check_nan) && (v != v))
				continue;
			sum += v;
			counter++;
		}
	}
	S	result = sum / counter;
	return result;
}

template<typename T, typename S>
T	Mean<T, S>::operator()(const ConstImageAdapter<T>& image) {
	return (T)filter(image);
}

/**
 * \brief Filter that finds the variance of an image
 */
template<typename T, typename S>
class Variance : public Mean<T, S> {
public:
	Variance() { }
	virtual S	filter(const ConstImageAdapter<T>& image);
	virtual T	operator()(const ConstImageAdapter<T>& image);
};

template<typename T, typename S>
S	Variance<T, S>::filter(const ConstImageAdapter<T>& image) {
	S	m = Mean<T, S>::filter(image);
	// the rest of the code is concerned with computing the
	// quadratic mean

	S	sum = 0;
	size_t	counter = 0;
	bool	check_nan = std::numeric_limits<T>::has_quiet_NaN;
	ImageSize	size = image.getSize();
	for (int x = 0; x < size.width(); x++) {
		for (int y = 0; y < size.height(); y++) {
			T	v = image.pixel(x, y);
			// skip NaNs
			if ((check_nan) && (v != v))
				continue;
			
			sum += (v - m) * (v - m);
			counter++;
		}
	}
	S	var = sum / counter;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "var = %f", var);
	return var;
}

template<typename T, typename S>
T	Variance<T, S>::operator()(const ConstImageAdapter<T>& image) {
	return (T)filter(image);
}

/**
 * \brief Filters that finds the mean of the various color channels
 */
template<typename T, typename S>
class MosaicMean : public Mean<T, S> {
protected:
	typedef enum color_e {
		R = 0, Gr = 1, B = 2, Gb = 3
	} color_type;
	color_type	color;
	ImagePoint	origin(const MosaicType::mosaic_type& mosaic) {
		MosaicType	m(mosaic);
		switch (color) {
		case R:		return m.red();
		case Gr:	return m.greenr();
		case B:		return m.blue();
		case Gb:	return m.greenb();
		}
		throw std::runtime_error("compiler error, should not get here");
	}
public:
	MosaicMean(color_type _color) : color(_color) { }
	virtual S	mean(const ConstImageAdapter<T>& image,
				const MosaicType::mosaic_type& mosaic) {
		if (mosaic & 0x8) {
			throw std::logic_error("not a mosaic image");
		}
		Subgrid	grid(origin(mosaic), ImageSize(2, 2));
		astro::adapter::ConstSubgridAdapter<T>	subimage(image, grid);
		Mean<T, S>	m;
		return m.filter(image);
	}
};

template<typename T, typename S>
class MeanR : public MosaicMean<T, S> {
public:
	MeanR() : MosaicMean<T, S>(MosaicMean<T, S>::R) { }
};

template<typename T, typename S>
class MeanGr : public MosaicMean<T, S> {
public:
	MeanGr() : MosaicMean<T, S>(MosaicMean<T, S>::Gr) { };
};

template<typename T, typename S>
class MeanB : public MosaicMean<T, S> {
public:
	MeanB() : MosaicMean<T, S>(MosaicMean<T, S>::B) { };
};

template<typename T, typename S>
class MeanGb : public MosaicMean<T, S> {
public:
	MeanGb() : MosaicMean<T, S>(MosaicMean<T, S>::Gb) { };
};

/**
 * \brief Filter that finds the median of an image
 */

template<typename T, typename S>
class Median : public PixelTypeFilter<T, S> {
	enum { N = 4 };
	T	upper_limit;
	T	lower_limit;

	T	median(const ConstImageAdapter<T>& image,
			const T& left, const T& right);
public:
	Median() { }

	virtual T	operator()(const ConstImageAdapter<T>& image);
	virtual S	filter(const ConstImageAdapter<T>& image);
};

template<typename T, typename S>
T	Median<T, S>::operator()(const ConstImageAdapter<T>& image) {
	// compute the maximum value that we have to consider.
	// this depends on whether we have a floating point type,
	// in which case we have to compute the maximum and minimum,
	// or whether we have an integer type, in which case we
	// can use std::numeric_limits
	T	lower_limit;
	T	upper_limit;
	// 
	if (std::numeric_limits<T>::is_integer) {
		lower_limit = 0;
		upper_limit = std::numeric_limits<T>::max();
	} else {
		Min<T, S>	minfilter;
		lower_limit = minfilter(image);
		Max<T, S>	maxfilter;
		upper_limit = maxfilter(image);
	}
	T	result = median(image, lower_limit, upper_limit);
	return result;
}

template<typename T, typename S>
S	Median<T, S>::filter(const ConstImageAdapter<T>& image) {
	return (S)this->operator()(image);
}

template<typename T, typename S>
T	Median<T, S>::median(const ConstImageAdapter<T>& image,
		const T& left, const T& right) {
	ImageSize	size = image.getSize();
	size_t	pixels = size.getPixels();
#if 0
std::cout << "left: " << (unsigned int)left << ", right: "
	<< (unsigned int)right << std::endl;
#endif
	unsigned long	count[N + 1];
	T	limits[N + 1];
	int	delta = 0;
	// reset all the limit values 
	for (unsigned int i = 0; i < N + 1; i++) {
		count[i] = 0;
		limits[i] = left + (i * (right - left)) / N;
		if (i > 0) {
			int	d = limits[i] - limits[i - 1];
			if (d > delta) {
				delta = d;
			}
		}
	}

	// count the number of values 
	for (int x = 0; x < size.width(); x++) {
		for (int y = 0; y < size.height(); y++) {
			T	v = image.pixel(x, y);
			for (unsigned int i = 0; i < N + 1; i++) {
				if (v <= limits[i]) {
					count[i]++;
				}
			}
		}
	}

	// terminal condition, if the delta <= 1, then we have
	// enough resolution to determine the median
	if (delta == 0) {
		return limits[0];
	}
	if (delta <= 1) {
		for (unsigned int i = 1; i < N + 1; i++) {
			if (((2 * count[i - 1]) < pixels)
				&& (pixels <= (2 * count[i]))) {
				return (limits[i - 1] + limits[i]) / 2;
			}
		}
	}

	// now we know how many values are in each bucket
#if 0
	for (int i = 0; i < N + 1; i++) {
		std::cout << "limit: " << limits[i]
			<< ", count: " << count[i] << std::endl;
	}
#endif
	// now find out in which interval we have to expect the
	// median
	
	if ((2 * count[N]) < pixels) {
		return median(image, limits[N], upper_limit);
	}
	if (pixels <= (2 * count[0])) {
		return median(image, 0, limits[0]);
	}
	for (unsigned int i = 1; i < N + 1; i++) {
		if (((2 * count[i - 1]) < pixels)
			&& (pixels <= (2 * count[i]))) {
			return median(image, limits[i - 1], limits[i]);
		}
	}
	throw std::logic_error("error in median computation");
}

/**
 * \brief Figure of Merit for autofocus
 *
 * This filter computes the integral of the value times the laplacian
 * of the image function. This is the L^2-norm of the first derivative.
 * The larger in absolute value, the better the focus. 
 */
template<typename Pixel>
class FocusFOM : public PixelTypeFilter<Pixel, double> {
	bool	diagonal;
	double	scale;
public:
	FocusFOM(bool _diagonal = false, double _scale = 1)
		: diagonal(_diagonal), scale(_scale) {
	}

	virtual double	filter(const ConstImageAdapter<Pixel>& image) {
		astro::adapter::FocusFOMAdapter<Pixel>	foa(image, diagonal);
		ImageSize	size = foa.getSize();
		double	result = 0;
		for (int x = 0; x < size.width(); x++) {
			for (int y = 0; y < size.height(); y++) {
				double	l = foa.pixel(x, y);
				// skip NaNs
				if (l == l) {
					result += l;
				}
			}
		}
		return scale * result;
	}

	virtual Pixel	operator()(const ConstImageAdapter<Pixel>& image) {
		Pixel	result = this->filter(image);
		return result;
	}
};

/**
 * \brief Image masking operations
 *
 * Masking operations are used to apply windowing functions or to black out
 * parts of an image the we don't want to see. 
 * a common base class.
 */
template<typename Pixel>
class Mask {
	MaskingFunction&	maskingfunction;
public:
	Mask(MaskingFunction& _maskingfunction)
		: maskingfunction(_maskingfunction) {
	}
	void	operator()(Image<Pixel>& image);
};

template<typename Pixel>
void    Mask<Pixel>::operator()(Image<Pixel>& image) {
	for (int x = 0; x < image.size().width(); x++) {
		for (int y = 0; y < image.size().height(); y++) {
			Pixel   v = image.pixel(x, y);
			v = maskingfunction(x, y) * v;
			image.pixel(x, y) = v;
		}
	}
}

/**
 * \brief Full Width at Half Maximum computation classes
 */
template<typename Pixel>
class FWHM : PixelTypeFilter<Pixel, double> {
	ImagePoint	point;
	int	r;
public:
	FWHM(const ImagePoint& _point, int _r) : point(_point), r(_r) {
	}
	virtual Pixel	operator()(const ConstImageAdapter<Pixel>& image) {
		return this->filter(image);
	}
	virtual double	filter(const ConstImageAdapter<Pixel>& image);
};

template<typename Pixel>
double	FWHM<Pixel>::filter(const ConstImageAdapter<Pixel>& image) {
	// first define the area where we should see the maximum
	ImagePoint	center(point.x() - r, point.y() - r);
	ImageRectangle	rectangle(center, ImageSize(2 * r + 1, 2 * r + 1));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for maximum in %s",
		rectangle.toString().c_str());
	astro::adapter::WindowAdapter<Pixel>	wa(image, rectangle);

	// locate the maximum in a rectangle around the point
	Max<Pixel, double>	m;
	double	maxvalue = m.filter(wa);
	ImagePoint	target = m.getPoint();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found maximum %f at %s",
		(double)maxvalue, target.toString().c_str());
	
	// find pixels that are above half maximum
	double	halfmax = maxvalue / 2;
	unsigned int	maxradius = trunc((r + 1) * 1.43);
	unsigned int	rhist[maxradius];
	for (unsigned int k = 0; k < maxradius; k++) {
		rhist[k] = 0;
	}
	for (int x = 0; x < wa.getSize().width(); x++) {
		for (int y = 0; y < wa.getSize().height(); y++) {
			if (wa.pixel(x, y) > halfmax) {
				unsigned int	k
					= trunc(hypot(x - target.x(), y - target.y()));
				if (k < maxradius) {
					rhist[k]++;
				}
			}
		}
	}
	
	// display the radius histogram
	for (unsigned int k = 0; k < maxradius; k++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "rhist[%03u] = %u",
			k, rhist[k]);
	}

	// find the maximum in the histogram
	unsigned int	maxr = 0;
	unsigned int	maxrh = 0;
	for (unsigned int k = 0; k < maxradius; k++) {
		if (rhist[k] > maxrh) {
			maxr = k;
			maxrh = rhist[k];
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "maximum %u at %u", maxrh, maxr);

	// fint the point where it first drops below half the maximum
	double	maxrh2 = maxrh / 2;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "half maximum: %f", maxrh2);
	unsigned int	hm;
	for (hm = maxr; hm < maxradius; hm++) {
		if (rhist[hm] == maxrh2) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "half max at %u", hm);
			return hm;
		}
		if (rhist[hm] < maxrh2) {
			break;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "drop off to half maximum: %u", hm);

	// interpolate the radius between hm and the previous point
	double	dx = (maxrh2 - rhist[hm - 1]) / (rhist[hm - 1] - rhist[hm]);
	double	fwhm = hm - dx;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "dx = %f, fwhm = %f", dx, fwhm);

	// return value
	return fwhm;
}

/**
 * \brief New implementation using Miniball algorithm
 *
 * This filter first determines the maximum value within the circle and
 * then collects all pixels within the circle that have a pixel value larger
 * than half maximum. Then the MiniBall algorithm is used to find the diameter.
 */
template<typename Pixel>
class FWHM2 : PixelTypeFilter<Pixel, double> {
	ImagePoint	point;
	unsigned int	r;
public:
	FWHM2(const ImagePoint& _point, unsigned int _r)
		: point(_point), r(_r) {
	}
	virtual Pixel	operator()(const ConstImageAdapter<Pixel>& image) {
		return this->filter(image);
	}
	virtual double	filter(const ConstImageAdapter<Pixel>& image);
	FWHMInfo	filter_extended(const ConstImageAdapter<Pixel>& image);
};

extern double	MinRadius(const std::list<ImagePoint>& points, Point& center);
extern double	MinRadius(const std::list<ImagePoint>& points, ImagePoint& center);
extern double	MinRadius(const std::list<ImagePoint>& points);

template<typename Pixel>
double	FWHM2<Pixel>::filter(const ConstImageAdapter<Pixel>& image) {
	return filter_extended(image).radius;
}

template<typename Pixel>
FWHMInfo	FWHM2<Pixel>::filter_extended(const ConstImageAdapter<Pixel>& image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "dowing FWHM2 from %s image",
		image.getSize().toString().c_str());
	if (!image.getSize().contains(point)) {
		throw std::runtime_error("point is outside image");
	}
	int	width = image.getSize().width();
	int	height = image.getSize().height();

	// first ensure that we take a radius small enough to fit inside
	// the image rectangle
	int	radius = r;
	if (point.x() < radius) {
		radius = point.x();
	}
	if (point.x() + radius >= width) {
		radius = width - point.x() - 1;
	}
	if (point.y() < radius) {
		radius = point.y();
	}
	if (point.y() + radius >= height) {
		radius = height - point.y() - 1;
	}

	// find the maximum value in the area defined by point and radius
	ImagePoint	lowerleft(point.x() - radius, point.y() - radius);
	ImageRectangle	rectangle(lowerleft,
				ImageSize(2 * radius + 1, 2 * radius + 1));
	if (!image.getSize().bounds(rectangle)) {
		std::string	msg = stringprintf("search rectangle %s does "
			"not fit image rectangle %s",
			rectangle.toString().c_str(),
			image.getSize().toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for maximum in %s",
		rectangle.toString().c_str());
	astro::adapter::WindowAdapter<Pixel>	wa(image, rectangle);

	// prepare the result
	FWHMInfo	result;

	// locate the maximum in a rectangle around the point
	Max<Pixel, double>	m;
	double	maxvalue = m.filter(wa);
	result.maxvalue = maxvalue;

	// the maximum point we have found is with respect to the window,
	// but that was a restriction only for finding the maximum. So
	// we now compute the target point relative to the whole image
	ImagePoint	target = lowerleft + m.getPoint();
	result.maxpoint = target;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found maximum %f at %s",
		(double)maxvalue, target.toString().c_str());

	// collect points that have pixel value > half maximum
	astro::adapter::LevelMaskAdapter<Pixel>	lma(image, maxvalue / 2.);
	ImagePtr	levelmask(new Image<unsigned char>(lma));

	// extract the connected component of this levelmask
	ImagePtr	connected = ConnectedComponent(target)(levelmask);
	result.mask = connected;
	Image<unsigned char>	*conn
		= dynamic_cast<Image<unsigned char> *>(&*connected);

	// add points in connected component to the list
	std::list<ImagePoint>	points;
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			if (conn->pixel(x, y)) {
				points.push_back(ImagePoint(x, y));
			}
		}
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d points", points.size());
	
	// now use Miniball to get access to the 
	result.radius = MinRadius(points, result.center);
	return result;
}

/**
 * \brief WhiteBalance computation class
 *
 * The WhiteBalance class computes average pixel densities and can be used
 * as a start for color correction.
 */
template<typename Pixel>
class WhiteBalance {
public:
	WhiteBalance() { }
	RGB<double>	filter(const ConstImageAdapter<RGB<Pixel> >& image)
				const;
};

template<typename Pixel>
RGB<double>	WhiteBalance<Pixel>::filter(
	const ConstImageAdapter<RGB<Pixel> >& image) const {
	double	L = 0;
	double	R = 0;
	double	G = 0;
	double	B = 0;
	unsigned int	m = 0;
	int	width = image.getSize().width();
	int	height = image.getSize().height();
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			RGB<Pixel>	v = image.pixel(x, y);
			double	l = v.luminance();
			L += l * l;
			v = v.colorcomponents();
//debug(LOG_DEBUG, DEBUG_LOG, 0, "%.3f, %.3f, %.3f", v.R, v.G, v.B);
			R += v.R * l;
			G += v.G * l;
			B += v.B * l;
			m++;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "L = %.3f, R = %.3f, G = %.3f, B = %.3f",
		L, R, G, B);
	RGB<double>	result((L - R) / m, (L - G) / m, (L - B) / m);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%.3f, %.3f, %.3f",
		result.R, result.G, result.B);
	result = result / result.luminance();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%.3f, %.3f, %.3f",
		result.R, result.G, result.B);
	return result;
}

/**
 * \brief Filter that computes the sum of an image
 */
template<typename Pixel>
class Sum {
public:
	Sum() { }
	double	filter(const ConstImageAdapter<Pixel>& image) const;
};

template<typename Pixel>
double	Sum<Pixel>::filter(const ConstImageAdapter<Pixel>& image) const {
	double	sum = 0;
	int	width = image.getSize().width();
	int	height = image.getSize().height();
	bool	check_nan = std::numeric_limits<Pixel>::has_quiet_NaN;
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			Pixel	v = image.pixel(x, y);
			if ((check_nan) && (v != v))
				continue;
			sum += v;
		}
	}
	return sum;
}

/**
 * \brief Base class for general filter
 */
template<typename T, typename S>
class GeneralFilter {
public:
	virtual S	operator()(const ConstImageAdapter<T>& image) = 0;
};

/**
 * \brief Find the peak in an image
 */
class PeakFinder : public GeneralFilter<double, Point> {
	ImagePoint	_approximate;
	int	_radius;
	int	above(const ConstImageAdapter<double>& image, double v);
	double	threshold(const ConstImageAdapter<double>& image,
			double minvalue, double maxvalue);
	std::pair<Point, double>	centroid(
			const ConstImageAdapter<double>& image,
			double threshold);
public:
	PeakFinder(const ImagePoint& approximate, int radius);
	virtual Point	operator()(const ConstImageAdapter<double>& image);
	std::pair<Point, double>	peak(const ConstImageAdapter<double>& image);
};

/**
 * \brief Filter to compute the centroid of a group of pixels
 *
 * This filter requires that the Pixel type T can be converted to double
 * (all scalar pixels as well as the RGB and YUYV pixels have this property)
 */
template<typename T>
class CentroidFilter : public GeneralFilter<T, Point> {
	ImagePoint	_approximate;
	double		_r;
public:
	CentroidFilter(ImagePoint approximate, double r)
		: _approximate(approximate), _r(r) {
	}
	virtual Point	operator()(const ConstImageAdapter<T>& image) {
		adapter::TypeConversionAdapter<T>	da(image);
		PeakFinder	pf(_approximate, ceil(_r));
		return pf(da);
	}
};

/**
 * \brief Find center of gravity
 */
class CGFilter : public GeneralFilter<double, Point> {
	double	_radius;
public:
	CGFilter(double radius) : _radius(radius) { }
	virtual Point	operator()(const ConstImageAdapter<double>& image);
};

} // namespace filter
} // namespace image
} // namespace astro

#endif /* _AstroFilter_h */
