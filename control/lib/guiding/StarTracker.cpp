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
		return sd(rectangle);					\
	}								\
}

Point	findstar(ImagePtr image, const ImageRectangle& rectangle, int /* k */) {
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

StarTracker::StarTracker(const Point& point,
	const ImageRectangle& rectangle, int k)
	: _point(point), _rectangle(rectangle), _k(k) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing a star tracker %s, %s, %d",
		point.toString().c_str(), rectangle.toString().c_str(), k);
}

Point	StarTracker::operator()(ImagePtr newimage) {
	// find the star on the new image, these coordinates are relative
	// to the actual image
	Point	newpoint = findstar(newimage, _rectangle, _k);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new point: %s, tracking point: %s, origin: %s",
		newpoint.toString().c_str(), _point.toString().c_str(),
		newimage->getFrame().origin().toString().c_str());
	// now we have to check whether the image is actually only a
	// subframe, and correct newpoint for its offset. This way we
	// get the star in absolute coordinates
	newpoint = newpoint + newimage->getFrame().origin();
	Point	offset = newpoint - _point;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "absolute: %s, offset: %s",
		newpoint.toString().c_str(), offset.toString().c_str());

	// new compute the offset
	return dithered(newpoint - _point);
}

std::string	StarTracker::toString() const {
	std::ostringstream	out;
	out << *this;
	return out.str();
}

std::ostream&	operator<<(std::ostream& out, const StarTracker& tracker) {
	out << tracker.point();
	out << "/";
	out << tracker.rectangle();
	out << "/";
	out << tracker.k();
	return out;
}

std::istream&	operator>>(std::istream& in, StarTracker& tracker) {
	Point	p;
	astro::image::ImageRectangle	r;
	int	k;
	in >> p;
	absorb(in, '/');
	in >> r;
	absorb(in, '/');
	in >> k;
	tracker.point(p);
	tracker.rectangle(r);
	tracker.k(k);
	return in;
}

} // namespace guiding
} // namespace astro
