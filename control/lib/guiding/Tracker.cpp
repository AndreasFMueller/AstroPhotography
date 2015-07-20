/*
 * Tracker.cpp -- classes implementing star trackers
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>

#include <AstroGuiding.h>
#include <AstroUtils.h>
#include <sstream>

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
		return sd(rectangle, k);				\
	}								\
}

Point	findstar(ImagePtr image, const ImageRectangle& rectangle,
		unsigned int k) {
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
	const ImageRectangle& rectangle, unsigned int k)
	: _point(point), _rectangle(rectangle), _k(k) {
}

Point	StarTracker::operator()(ImagePtr newimage) const {
	// find the star on the new image
	Point	newpoint = findstar(newimage, _rectangle, _k);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new point: %s, tracking point: %s",
		newpoint.toString().c_str(), _point.toString().c_str());
	return newpoint - _point;
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

#define	phasetracker_construct(Pixel)					\
{									\
	Image<Pixel>	*imagep						\
		= dynamic_cast<Image<Pixel > *>(&*_image);		\
	if (NULL != imagep) {						\
		LuminanceAdapter<Pixel, double>	la(*imagep);		\
		image = ImagePtr(new Image<double>(la));		\
		return;							\
	}								\
}

PhaseTracker::PhaseTracker(ImagePtr _image) {
	phasetracker_construct(unsigned char);
	phasetracker_construct(unsigned short);
	phasetracker_construct(unsigned int);
	phasetracker_construct(unsigned long);
	phasetracker_construct(float);
	phasetracker_construct(double);
	phasetracker_construct(RGB<unsigned char>);
	phasetracker_construct(RGB<unsigned short>);
	phasetracker_construct(RGB<unsigned int>);
	phasetracker_construct(RGB<unsigned long>);
	phasetracker_construct(RGB<float>);
	phasetracker_construct(RGB<double>);
	phasetracker_construct(YUYV<unsigned char>);
	phasetracker_construct(YUYV<unsigned short>);
	phasetracker_construct(YUYV<unsigned int>);
	phasetracker_construct(YUYV<unsigned long>);
	phasetracker_construct(YUYV<float>);
	phasetracker_construct(YUYV<double>);
	throw std::runtime_error("cannot track this image type");
}

#define	phasetracker_typed(Pixel)					\
{									\
	Image<Pixel>	*newimagep					\
		= dynamic_cast<Image<Pixel > *>(&*newimage);		\
	if (NULL != newimagep) {					\
		LuminanceAdapter<Pixel, double>	newla(*newimagep);	\
		Image<double>	*imagep					\
			= dynamic_cast<Image<double> *>(&*image);	\
		PhaseCorrelator	pc;					\
		return pc(*imagep, newla).first;			\
	}								\
}

Point	PhaseTracker::operator()(ImagePtr newimage)
	const {
	phasetracker_typed(unsigned char);
	phasetracker_typed(unsigned short);
	phasetracker_typed(unsigned int);
	phasetracker_typed(unsigned long);
	phasetracker_typed(float);
	phasetracker_typed(double);
	phasetracker_typed(RGB<unsigned char>);
	phasetracker_typed(RGB<unsigned short>);
	phasetracker_typed(RGB<unsigned int>);
	phasetracker_typed(RGB<unsigned long>);
	phasetracker_typed(RGB<float>);
	phasetracker_typed(RGB<double>);
	phasetracker_typed(YUYV<unsigned char>);
	phasetracker_typed(YUYV<unsigned short>);
	phasetracker_typed(YUYV<unsigned int>);
	phasetracker_typed(YUYV<unsigned long>);
	phasetracker_typed(YUYV<float>);
	phasetracker_typed(YUYV<double>);
	throw std::runtime_error("cannot track this image type");
}

std::string	PhaseTracker::toString() const {
	std::string	info = stringprintf("PhaseTracker on %s image",
		image->size().toString().c_str());
	return info;
}

} // namespace guiding
} // namespace astro
