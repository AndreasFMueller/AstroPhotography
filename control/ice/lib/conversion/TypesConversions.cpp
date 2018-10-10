/*
 * TypesConversions.cpp -- conversions between ice and astro, types
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <IceConversions.h>
#include <includes.h>
#include <AstroIO.h>
#include <AstroUtils.h>

namespace snowstar {

// time conversions
time_t	converttime(double timeago) {
	time_t	now;
	time(&now);
	time_t	result = now - timeago;
	return result;
}

double	converttime(time_t t) {
	time_t	now;
	time(&now);
	return now - t;
}
 
struct timeval	converttimeval(double timeago) {
	struct timeval	result;
	gettimeofday(&result, NULL);
	result.tv_sec -= (int)trunc(timeago);
	result.tv_usec -= (int)(1000000 * (timeago - trunc(timeago)));
	while (result.tv_usec < 0) {
		result.tv_sec -= 1;
		result.tv_usec += 1000000;
	}
	while (result.tv_usec > 1000000) {
		result.tv_sec += 1;
		result.tv_usec -= 1000000;
	}
	return result;
}

double	converttimeval(struct timeval t) {
	struct timeval	now;
	gettimeofday(&now, NULL);
	return (now.tv_sec - t.tv_sec) + (now.tv_usec - t.tv_usec) / 1000000.;
}


// Types
ImagePoint	convert(const astro::image::ImagePoint& point) {
	ImagePoint	result;
	result.x = point.x();
	result.y = point.y();
	return result;
}

astro::image::ImagePoint	convert(const ImagePoint& point) {
	return astro::image::ImagePoint(point.x, point.y);
}

ImageSize	convert(const astro::image::ImageSize& size) {
	ImageSize	result;
	result.width = size.width();
	result.height = size.height();
	return result;
}

astro::image::ImageSize	convert(const ImageSize& size) {
	return astro::image::ImageSize(size.width, size.height);
}

ImageRectangle	convert(const astro::image::ImageRectangle& rectangle) {
	ImageRectangle	result;
	result.origin = convert(rectangle.origin());
	result.size = convert(rectangle.size());
	return result;
}

astro::image::ImageRectangle	convert(const ImageRectangle& rectangle) {
	return astro::image::ImageRectangle(convert(rectangle.origin),
			convert(rectangle.size));
}

Point	convert(const astro::Point& point) {
	Point	result;
	result.x = point.x();
	result.y = point.y();
	return result;
}

astro::Point	convert(const Point& point) {
	return astro::Point(point.x, point.y);
}

astro::RaDec	convert(const RaDec& radec) {
	astro::Angle	ra;
	ra.hours(radec.ra);
	astro::Angle	dec;
	dec.degrees(radec.dec);
	return astro::RaDec(ra, dec);
}

RaDec	convert(const astro::RaDec& radec) {
	RaDec	result;
	result.ra = radec.ra().hours();
	if (radec.dec() > astro::Angle(M_PI)) {
		result.dec = (radec.dec() - astro::Angle(2 * M_PI)).degrees();
	} else {
		result.dec = radec.dec().degrees();
	}
	return result;
}

astro::AzmAlt	convert(const AzmAlt& azmalt) {
	astro::Angle	azm;
	azm.degrees(azmalt.azm);
	astro::Angle	alt;
	alt.degrees(azmalt.alt);
	return astro::AzmAlt(azm, alt);
}

AzmAlt	convert(const astro::AzmAlt& azmalt) {
	AzmAlt	result;
	result.azm = azmalt.azm().degrees();
	result.alt = azmalt.alt().degrees();
	return result;
}

astro::LongLat	convert(const LongLat& longlat) {
	astro::Angle	longitude;
	longitude.degrees(longlat.longitude);
	astro::Angle	latitude;
	latitude.degrees(longlat.latitude);
	return astro::LongLat(longitude, latitude);
}

LongLat	convert(const astro::LongLat& longlat) {
	LongLat	result;
	result.longitude = longlat.longitude().degrees();
	result.latitude = longlat.latitude().degrees();
	return result;
}

} // namespace snowstar
