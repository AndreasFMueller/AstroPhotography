/*
 * StarDetectorBase.cpp -- base class for a class that finds star positions
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <AstroAdapter.h>
#include <AstroFilter.h>

using namespace astro::image;

namespace astro {
namespace guiding {

/**
 * \brief Find the star withing the area of Interest
 *
 * This method finds an approximate (pixel precision) location of the
 * star by looking for a weighted maximum within the area of interest.
 */
StarDetectorBase::findResult	StarDetectorBase::findStar(
			const ConstImageAdapter<double>& _image,
			const ImageRectangle& areaOfInterest) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "areaOfInterest: %s",
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

	// compute the real coordinates of the maximum
	result.point = areaOfInterest.subimage(result.point);

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
 */
double	StarDetectorBase::radius(const ConstImageAdapter<double>& _image,
		const ImagePoint& where) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "find FWHM radius around %s",
		where.toString().c_str());
	// find out how close to the border we are
	int	k = _image.getSize().borderDistance(where);
	if (k < 3) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"%s is too close to the border of %s",
			where.toString().c_str(),
			_image.getSize().toString().c_str());
		throw std::runtime_error("too close to border");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "border distance: %d", k);

	// get the value at the where position
	double	maxvalue = _image.pixel(where) / 2;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "maximum value is %.3f", maxvalue);

	// radius larger than 20 is almost surely a insufficiently
	// focused star, so we only consider points sufficiently close
	if (k > 20) { k = 20; }
	bool	smaller[k + 1];
	for (int i = 0; i <= k; i++) { smaller[i] = true; }
	
	ImageRectangle	rectangle(where - ImagePoint(k, k),
		ImageSize(2 * k + 1, 2 * k + 1));
	adapter::WindowAdapter<double>	wa(_image, rectangle);
	for (int x = 0; x <= 2 * k; x++) {
		for (int y = 0; y <= 2 * k; y++) {
			double	value = wa.pixel(x, y);
			int	r = ceil(hypot(x - k, y - k));
			if ((r <= k) && (value > maxvalue)) {
				smaller[r] &= false;
			}
		}
	}

	// find the first index where 
	for (int i = 0; i <= k; i++) {
		if (smaller[i]) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "found radius: %d", i);
			return i;
		}
	}

	// default: consider all 
	debug(LOG_DEBUG, DEBUG_LOG, 0, "using default radius: %d", k);
	return k;
}

static const int	radius_multiplier = 1;

/**
 * \brief Find the exact star position
 *
 * This method computes the exact star position by averaging the star
 * locations in an area around the maximum found by the findStar method.
 */
Point	StarDetectorBase::operator()(const ConstImageAdapter<double>& image,
		const ImageRectangle& rectangle) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "find star in rectangle %s",
		rectangle.toString().c_str());
	// first find the approximate position
	StarDetectorBase::findResult	location = findStar(image, rectangle);
	ImagePoint	approximate = location.point;

	// create a new adapter that subtracts the background, we use that
	// for the following computations
	adapter::AddConstantAdapter<double, double>	bgimage(image,
		-location.background);

	// determine the radius of points to include in the averaging
	double	r = radius(bgimage, approximate);

	// get the border distance
	int	k = bgimage.getSize().borderDistance(approximate);
	if ((radius_multiplier * r) > k) {
		std::string	cause = stringprintf("not enough room for "
			"averaging: fwhm=%d, border=%d", r, k);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw std::runtime_error(cause);
	}
	r = radius_multiplier * r;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "averaging radius %.2f around %s", r,
		approximate.toString().c_str());
	// make the radius large engough for the PeakFinder to work
	if (r < 5) {
		r = 5;
	}

	// now use the CentroidFilter to get the centroid
	image::filter::CentroidFilter<double>	cf(approximate, r);
	Point	centroid = cf(bgimage);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "centeroid found: %s",
		centroid.toString().c_str());
	return centroid;
}

} // namespace guiding
} // namespace astro
