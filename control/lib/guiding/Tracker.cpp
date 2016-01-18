/*
 * Tracker.cpp -- classes implementing star trackers
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

//////////////////////////////////////////////////////////////////////
// Implementation of the Phase Tracker
//////////////////////////////////////////////////////////////////////

PhaseTracker::PhaseTracker() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing a phase tracker");
	image = NULL;
}

#define	phasetracker_construct(Pixel)					\
{									\
	Image<Pixel>	*imagep						\
		= dynamic_cast<Image<Pixel > *>(&*newimage);		\
	if (NULL != imagep) {						\
		LuminanceAdapter<Pixel, double>	la(*imagep);		\
		image = new Image<double>(la);				\
		imageptr = ImagePtr(image);				\
		return Point(0, 0);					\
	}								\
}

#define	phasetracker_typed(Pixel)					\
{									\
	Image<Pixel>	*imagep						\
		= dynamic_cast<Image<Pixel > *>(&*newimage);		\
	if (NULL != imagep) {						\
		LuminanceAdapter<Pixel, double>	la(*imagep);		\
		PhaseCorrelator	pc;					\
		return pc(*image, la).first;				\
	}								\
}

Point	PhaseTracker::operator()(ImagePtr newimage) {
	if (!imageptr) {
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
			(image) ? image->size().toString().c_str()
				: "(undefined)");
	return info;
}

//////////////////////////////////////////////////////////////////////
// Implementation of the Differential Phase Tracker
//////////////////////////////////////////////////////////////////////

DifferentialPhaseTracker::DifferentialPhaseTracker() {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"constructing a differential phase tracker");
	image = NULL;
}

#define	diffphasetracker_construct(Pixel)				\
{									\
	Image<Pixel>	*imagep						\
		= dynamic_cast<Image<Pixel > *>(&*newimage);		\
	if (NULL != imagep) {						\
		LuminanceAdapter<Pixel, double>	la(*imagep);		\
		DerivativeNormAdapter<double>	da(la);			\
		image = new Image<double>(da);				\
		imageptr = ImagePtr(image);				\
		return Point(0, 0);					\
	}								\
}

#define	diffphasetracker_typed(Pixel)					\
{									\
	Image<Pixel>	*imagep						\
		= dynamic_cast<Image<Pixel > *>(&*newimage);		\
	if (NULL != imagep) {						\
		LuminanceAdapter<Pixel, double>	la(*imagep);		\
		DerivativeNormAdapter<double>	da(la);			\
		PhaseCorrelator	pc(false);				\
		return pc(*image, da).first;				\
	}								\
}

Point	DifferentialPhaseTracker::operator()(ImagePtr newimage) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getting offset from %s image",
		newimage->size().toString().c_str());
	if (!imageptr) {
		diffphasetracker_construct(unsigned char);
		diffphasetracker_construct(unsigned short);
		diffphasetracker_construct(unsigned int);
		diffphasetracker_construct(unsigned long);
		diffphasetracker_construct(float);
		diffphasetracker_construct(double);
		diffphasetracker_construct(RGB<unsigned char>);
		diffphasetracker_construct(RGB<unsigned short>);
		diffphasetracker_construct(RGB<unsigned int>);
		diffphasetracker_construct(RGB<unsigned long>);
		diffphasetracker_construct(RGB<float>);
		diffphasetracker_construct(RGB<double>);
		diffphasetracker_construct(YUYV<unsigned char>);
		diffphasetracker_construct(YUYV<unsigned short>);
		diffphasetracker_construct(YUYV<unsigned int>);
		diffphasetracker_construct(YUYV<unsigned long>);
		diffphasetracker_construct(YUYV<float>);
		diffphasetracker_construct(YUYV<double>);
		throw std::runtime_error("cannot track this image type");
	}
	diffphasetracker_typed(unsigned char);
	diffphasetracker_typed(unsigned short);
	diffphasetracker_typed(unsigned int);
	diffphasetracker_typed(unsigned long);
	diffphasetracker_typed(float);
	diffphasetracker_typed(double);
	diffphasetracker_typed(RGB<unsigned char>);
	diffphasetracker_typed(RGB<unsigned short>);
	diffphasetracker_typed(RGB<unsigned int>);
	diffphasetracker_typed(RGB<unsigned long>);
	diffphasetracker_typed(RGB<float>);
	diffphasetracker_typed(RGB<double>);
	diffphasetracker_typed(YUYV<unsigned char>);
	diffphasetracker_typed(YUYV<unsigned short>);
	diffphasetracker_typed(YUYV<unsigned int>);
	diffphasetracker_typed(YUYV<unsigned long>);
	diffphasetracker_typed(YUYV<float>);
	diffphasetracker_typed(YUYV<double>);
	throw std::runtime_error("cannot track this image type");
}

std::string	DifferentialPhaseTracker::toString() const {
	std::string	info
		= stringprintf("DifferentialPhaseTracker on %s image",
			(image) ? image->size().toString().c_str()
				: "(undefined)");
	return info;
}


} // namespace guiding
} // namespace astro
