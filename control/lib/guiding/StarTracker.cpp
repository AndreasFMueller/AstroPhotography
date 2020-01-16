/*
 * StarTracker.cpp -- classes implementing star trackers
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>

#include <AstroGuiding.h>
#include <AstroUtils.h>
#include <sstream>
#include <AstroDebug.h>

using namespace astro::image;
using namespace astro::image::transform;
using namespace astro::adapter;

namespace astro {
namespace guiding {

#define	findstar_typed(Pixel)						\
{									\
	Image<Pixel >	*imagep						\
		= dynamic_cast<Image<Pixel > *>(&*image);		\
	if (NULL != imagep) {						\
		StarDetector<Pixel >	sd(*imagep);			\
		Point	result = sd(searcharea);			\
		_processedImage = sd.analysis();			\
		return result;						\
	}								\
}

/**
 * \brief Type disambiguation function for finding a star
 *
 * \param image		the image to search for a star
 * \param searcharea	the search area to scan for the star
 */
Point	StarTracker::findstar(ImagePtr image,
		const ImageRectangle& searcharea) {
	findstar_typed(unsigned char);
	findstar_typed(unsigned short);
	findstar_typed(unsigned int);
	findstar_typed(unsigned long);
	findstar_typed(float);
	findstar_typed(double);
	findstar_typed(RGB<unsigned char>);
	findstar_typed(RGB<unsigned short>);
	findstar_typed(RGB<unsigned int>);
	findstar_typed(RGB<unsigned long>);
	findstar_typed(RGB<float>);
	findstar_typed(RGB<double>);
	findstar_typed(YUYV<unsigned char>);
	findstar_typed(YUYV<unsigned short>);
	findstar_typed(YUYV<unsigned int>);
	findstar_typed(YUYV<unsigned long>);
	findstar_typed(YUYV<float>);
	findstar_typed(YUYV<double>);
	throw std::runtime_error("cannot find star in this image type");
}

#undef findstar_typed
#define	findstar_typed(Pixel)						\
{									\
	Image<Pixel >	*imagep						\
		= dynamic_cast<Image<Pixel > *>(&*image);		\
	if (NULL != imagep) {						\
		StarDetector<Pixel >	sd(*imagep);			\
		Point	result = sd(searcharea);			\
		return result;						\
	}								\
}


Point	findstar(ImagePtr image, const ImageRectangle& searcharea) {
	findstar_typed(unsigned char);
	findstar_typed(unsigned short);
	findstar_typed(unsigned int);
	findstar_typed(unsigned long);
	findstar_typed(float);
	findstar_typed(double);
	findstar_typed(RGB<unsigned char>);
	findstar_typed(RGB<unsigned short>);
	findstar_typed(RGB<unsigned int>);
	findstar_typed(RGB<unsigned long>);
	findstar_typed(RGB<float>);
	findstar_typed(RGB<double>);
	findstar_typed(YUYV<unsigned char>);
	findstar_typed(YUYV<unsigned short>);
	findstar_typed(YUYV<unsigned int>);
	findstar_typed(YUYV<unsigned long>);
	findstar_typed(YUYV<float>);
	findstar_typed(YUYV<double>);
	throw std::runtime_error("cannot find star in this image type");
}

/**
 * \brief Construct a tracker
 *
 * \param trackingpoint		the point to track
 * \param searcharea		the rectangle within which to locate the star
 */
StarTracker::StarTracker(const Point& trackingpoint,
	const ImageRectangle& searcharea)
	: _trackingpoint(trackingpoint), _searcharea(searcharea) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing a star tracker "
		"trackingpoint=%s, searcharea=%s",
		trackingpoint.toString().c_str(),
		searcharea.toString().c_str());
}

/**
 * \brief Find the star to be tracked relative to the tracking point
 *
 * This method analyzes the new image and finds the current offset from 
 * the tracking point. This allows the guider to compute corrections.
 *
 * \param newimage	The image to locate the star in 
 */
Point	StarTracker::operator()(ImagePtr newimage) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "find star in %s image rectangle",
		newimage->getFrame().toString().c_str());
	// find the star on the new image, these coordinates are relative
	// to the actual image
	Point	newpoint = findstar(newimage, _searcharea);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new point: %s, tracking point: %s, "
		"origin: %s",
		newpoint.toString().c_str(), _trackingpoint.toString().c_str(),
		newimage->getFrame().origin().toString().c_str());

	// now we have to check whether the image is actually only a
	// subframe, and correct newpoint for its offset. This way we
	// get the star in absolute coordinates
	newpoint = newpoint + newimage->getFrame().origin();
	Point	offset = newpoint - _trackingpoint;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "absolute: %s, offset: %s",
		newpoint.toString().c_str(), offset.toString().c_str());

	// new compute the offset
	return dithered(newpoint - _trackingpoint);
}

std::string	StarTracker::toString() const {
	std::ostringstream	out;
	out << *this;
	return out.str();
}

std::ostream&	operator<<(std::ostream& out, const StarTracker& tracker) {
	out << tracker.trackingpoint();
	out << "/";
	out << tracker.searcharea();
	return out;
}

std::istream&	operator>>(std::istream& in, StarTracker& tracker) {
	Point	p;
	astro::image::ImageRectangle	r;
	in >> p;
	absorb(in, '/');
	in >> r;
	tracker.trackingpoint(p);
	tracker.searcharea(r);
	return in;
}

} // namespace guiding
} // namespace astro
