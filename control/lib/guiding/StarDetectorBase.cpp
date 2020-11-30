/*
 * StarDetectorBase.cpp -- base class for a class that finds star positions
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <AstroAdapter.h>
#include <AstroFilter.h>
#include <AstroTonemapping.h>
#include <AstroFilterfunc.h>
#include <AstroIO.h>
#include <AstroConfig.h>

using namespace astro::image;

namespace astro {
namespace guiding {

// search radius for hot pixel detection
config::ConfigurationKey	_hotpixel_radius_key(
	"guiding", "hotpixel", "radius");
config::ConfigurationRegister	_hotpixel_radius_registration(
	_hotpixel_radius_key,
	"radius in pixels to consider around a potential hot pixel");

// amplitude threshold for hot pixel detection
config::ConfigurationKey	_hotpixel_stddev_key(
	"guiding", "hotpixel", "stddev_multiplier");
config::ConfigurationRegister	_hotpixel_stddev_registration(
	_hotpixel_stddev_key,
	"number of std deviations of a value from the average for a pixel "
	"to be considered hot");

// maximum radius for averaging a star
config::ConfigurationKey	_stardetector_maxradius_key(
	"guiding", "stardetector", "maxradius");
config::ConfigurationRegister	_stardetector_maxradius_registration(
	_stardetector_maxradius_key,
	"maximum radius of pixels to average to find the centroid of a "
	"pixel (default 20 pixel)");

// minimum radius for fwhm stars
config::ConfigurationKey	_stardetector_minradius_key(
	"guiding", "stardetector", "minradius");
config::ConfigurationRegister	_stardetector_minradius_registration(
	_stardetector_minradius_key,
	"minimum radius of pixels to average to find the centroid of a "
	"pixel (5 pixel)");

/**
 * \brief Find the star withing the area of Interest
 *
 * This method finds an approximate (pixel precision) location of the
 * star by looking for a weighted maximum within the area of interest.
 */
StarDetectorBase::findResult	StarDetectorBase::findStar(
			const ConstImageAdapter<double>& _image,
			const ImageRectangle& areaOfInterest) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image size=%s, areaOfInterest: %s",
		_image.getSize().toString().c_str(),
		areaOfInterest.toString().c_str());
	findResult	result;

	// use the Weightingadapter
	adapter::WeightingAdapter	wa(_image, areaOfInterest);
	
	// use the maixmum filter to find the maximum point
	image::filter::Max<double, double>	maxfilter;
	double	maxValue = maxfilter(wa);
	result.point = maxfilter.getPoint();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "approximate star position %s, value %f",
		result.point.toString().c_str(), maxValue);

	// compute the minimum value
	image::filter::Min<double, double>	minfilter;
	result.background = minfilter(_image);

	// that's it
	return result;
}

/**
 * \brief Find the radius around the point to include
 *
 * This method first determines the maximum value, then finds out
 * how far away we have to go to find all values half the maximum value
 *
 * \param _image	the image to use to compute the radius
 * \param where		point inside the image adapter
 */
double	StarDetectorBase::radius(const ConstImageAdapter<double>& _image,
		const ImagePoint& where) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "find FWHM radius around %s, size=%s",
		where.toString().c_str(), _image.getSize().toString().c_str());
	// find out how close to the border we are
	int	bd = _image.getSize().borderDistance(where);
	if (bd < 3) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"%s is too close to the border of %s",
			where.toString().c_str(),
			_image.getSize().toString().c_str());
		throw std::runtime_error("too close to border");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "border distance: %d", bd);

	// get the value at the where position
	double	halfmaxvalue = _image.pixel(where) / 2;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "half maximum value is %.3f",
		halfmaxvalue);

	// radius larger than 20 is almost surely a insufficiently
	// focused star, so we only consider points sufficiently close
	int	maxradius = 20;
	config::ConfigurationPtr	config = config::Configuration::get();
	if (config->has(_stardetector_maxradius_key)) {
		maxradius = std::stoi(config->get(_stardetector_maxradius_key));
	}
	if (bd < maxradius) {
		maxradius = bd;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "using maxradius=%d", maxradius);

	// we now try to find out whether we have values larger than
	// the half maximum. We do this by constructing an array of booleans
	// that we then fill with false values whenever we find a larger
	// value
	bool	smaller[maxradius + 1];
	for (int i = 0; i <= maxradius; i++) {
		smaller[i] = true;
	}
	
	// scan a rectangle around the point 
	ImageRectangle	rectangle(where - ImagePoint(maxradius, maxradius),
			ImageSize(2 * maxradius + 1, 2 * maxradius + 1));
	adapter::WindowAdapter<double>	wa(_image, rectangle);
	for (int x = 0; x <= 2 * maxradius; x++) {
		for (int y = 0; y <= 2 * maxradius; y++) {
			double	value = wa.pixel(x, y);
			int	r = ceil(hypot(x - maxradius, y - maxradius));
			// if we are within the radius and the value is larger
			// remember that smaller[r] is false
			if ((r <= maxradius) && (value > halfmaxvalue)) {
				smaller[r] &= false;
			}
		}
	}

	// find the first index where smaller is true, which means that
	// all values at this distance were smaller
	for (int i = 0; i <= maxradius; i++) {
		if (smaller[i]) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "found radius: %d", i);
			return i;
		}
	}

	// default: consider all 
	debug(LOG_DEBUG, DEBUG_LOG, 0, "using default radius: %d", maxradius);
	return maxradius;
}

static const int	radius_multiplier = 1;
static int	imagecounter = 0;

/**
 * \brief Find the exact star position
 *
 * This method computes the exact star position by averaging the star
 * locations in an area around the maximum found by the findStar method.
 *
 * \param image			the image to work on
 * \param areaofinterest	the rectangle to scan for the star
 */
Point	StarDetectorBase::operator()(const ConstImageAdapter<double>& image,
		const ImageRectangle& areaofinterest) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "find star in rectangle %s in %s image",
		areaofinterest.toString().c_str(),
		image.getSize().toString().c_str());

	// clear the previous analysis image as it is no longer valid
	_analysis = NULL;
	_analysis_ptr.reset();

	// elimineate hot pixels
	adapter::HotPixelInterpolationAdapter<double>	hpia(image);

	// check whether we have special configuration for the hot pixel
	// detecter
	config::ConfigurationPtr	config = config::Configuration::get();
	if (config->has(_hotpixel_radius_key)) {
		hpia.search_radius(std::stoi(config->get(_hotpixel_radius_key)));
	}
	if (config->has(_hotpixel_stddev_key)) {
		hpia.stddev_multiplier(std::stoi(config->get(_hotpixel_stddev_key)));
	}

	// now build an image without hot pixels
	Image<double>	coolimage(hpia);

	// first find the approximate position inside the area of interest
	StarDetectorBase::findResult	location = findStar(coolimage,
						areaofinterest);
	ImagePoint	approximate = location.point;
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"approximate position %s, areaofinterest %s, background %f",
		approximate.toString().c_str(),
		areaofinterest.toString().c_str(),
		location.background);

	// create a new adapter that subtracts the background, we use that
	// for the following computations
	adapter::AddConstantAdapter<double, double>	bgimage(coolimage,
		-location.background);

	// draw a rescaled image
	drawImage(bgimage);
	drawHotpixels(hpia.bad_pixels());

	// determine the radius of points to include in the averaging
	double	r = radius(bgimage, approximate);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "fwhm radius: %f%s", r,
		(r > 15) ? ", very large! no star found?" : "");

	// make the radius large engough for the PeakFinder to work
	int	minradius = 5;
	if (config->has(_stardetector_minradius_key)) {
		minradius = std::stoi(config->get(_stardetector_minradius_key));
	}
	if (r < minradius) {
		r = minradius;
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"increased radius from %f to %d", r, minradius);
	}
	drawRadius(approximate, r);

	// draw the target cross
	double	l = image.getSize().width();
	if (image.getSize().height() > l) {
		l = image.getSize().height();
	}
	drawTarget(target(), l);

	// get the border distance
	int	bd = bgimage.getSize().borderDistance(approximate);
	if ((radius_multiplier * r) > bd) {
		std::string	cause = stringprintf("candidate %s too close "
			"to border for averaging: fwhm=%f, border=%d",
			approximate.toString().c_str(), r, bd);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw std::runtime_error(cause);
	}
	r = radius_multiplier * r;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "averaging radius %.2f around %s", r,
		approximate.toString().c_str());

	// now use the CentroidFilter to get the centroid
	image::filter::CentroidFilter<double>	cf(approximate, r);
	Point	centroid = cf(bgimage);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "centeroid found: %s",
		centroid.toString().c_str());

	// draw the centroid
	drawCentroid(centroid, 2 * r);

	// add metadata
	analysis()->setMetadata(
		io::FITSKeywords::meta(std::string("TARGETX"), target().x()));
	analysis()->setMetadata(
		io::FITSKeywords::meta(std::string("TARGETY"), target().y()));

	// for debugging, write image to debug directory
	try {
		std::string	filename = stringprintf("./debug/detector-%05d.fits",
					imagecounter++);
		io::FITSout	out(filename);
		out.setPrecious(false);
		out.write(analysis());
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot write image: %s",
			x.what());
	}

	// return the image
	return centroid;
}

/**
 * \brief Draw the luminance image in the analysis image
 *
 * \param image		the luminance image
 */
void	StarDetectorBase::drawImage(const ConstImageAdapter<double>& image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw the image");
	// find the maximum value
	image::filter::Max<double, double>	max;
	double	maximum = max(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "maximum: %f", maximum);

	// create an adapter that turns this maximum into the range 0-255
	double	scalefactor = 255 / maximum;
	adapter::RescalingAdapter<double>	ra(image, 0, scalefactor);

	// create a RGB<unsigned char> image from this
	adapter::ConvertingAdapter<RGB<unsigned char>, double>	converting(ra);

	_analysis = new Image<RGB<unsigned char> >(converting);
	_analysis_ptr = ImagePtr(_analysis);
}

/**
 * \brief Auxiliary function to draw a cross into the analysis image
 */
void	StarDetectorBase::drawCross(const ImagePoint& point, int length,
		const RGB<unsigned char>& pixel) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw cross length %d at %s",
		length, point.toString().c_str());
	int	x = point.x();
	int	y = point.y();
	int	w = _analysis->size().width();
	int	h = _analysis->size().height();
	int	xmin = x - length; if (xmin < 0) { xmin = 0; }
	int	xmax = x + length; if (xmax >= w) { xmax = w - 1; }
	int	ymin = y - length; if (ymin < 0) { ymin = 0; }
	int	ymax = y + length; if (ymax >= h) { ymax = h - 1; }
	if ((0 <= x) && (x <= w-1)) {
		for (int xx = xmin; xx <= xmax; xx++) {
			_analysis->pixel(xx, y) = pixel;
		}
	}
	if ((0 <= y) && (y <= h-1)) {
		for (int yy = ymin; yy <= ymax; yy++) {
			_analysis->pixel(x, yy) = pixel;
		}
	}
}

/**
 * \brief Draw the pixels as small crosses in the image
 *
 * \param hotpixels	coordinates of the hot pixels
 */
void	StarDetectorBase::drawHotpixels(const std::list<ImagePoint>& hotpixels) {
	RGB<unsigned char>	red((unsigned char)255, 0, 0);
	for (const ImagePoint& hotpixel : hotpixels) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "add hot pixel at %s",
			hotpixel.toString().c_str());
		drawCross(hotpixel, 1, red);
	}
}

/**
 * \brief Draw a circle for the averaging radius
 *
 * \param approximage
 * \param radius
 */
void	StarDetectorBase::drawRadius(const ImagePoint& approximate,
		double radius) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Fill circle of radius %f around %s",
		radius, approximate.toString().c_str());
	int	xmin = floor(approximate.x() - radius);
	int	ymin = floor(approximate.y() - radius);
	int	xmax = ceil(approximate.x() + radius);
	int	ymax = ceil(approximate.y() + radius);
	if (xmin < 0) { xmin = 0; }
	if (ymin < 0) { ymin = 0; }
	int	w = _analysis->size().width();
	int	h = _analysis->size().height();
	if (xmax >= w) { xmax = w - 1; }
	if (ymax >= h) { ymax = h - 1; }
	for (int x = xmin; x <= xmax; x++) {
		for (int y = ymin; y <= ymax; y++) {
			double	r = hypot(x - approximate.x(),
				y - approximate.y());
			if (r <= radius) {
				RGB<unsigned char> p = _analysis->pixel(x, y);
				p.B = 255;
				_analysis->pixel(x, y) = p;
			}
		}
	}
}

/**
 * \brief Draw the centroid
 *
 * \param centroid	the point
 */
void	StarDetectorBase::drawCentroid(const Point& centroid, double length) {
	int	l = length;
	ImagePoint	icentroid(centroid.x(), centroid.y());
	RGB<unsigned char>	green(0, (unsigned char)204, 0);
	drawCross(icentroid, l, green);
}

/**
 * \brief Draw the target cross in 
 *
 * \param target	target point to mark
 */
void	StarDetectorBase::drawTarget(const Point& target, double length) {
	int	l = length;
	ImagePoint	itarget(target.x(), target.y());
	RGB<unsigned char>  violett((unsigned char)204, 0, (unsigned char)204);
	drawCross(itarget, l, violett);
}

} // namespace guiding
} // namespace astro
