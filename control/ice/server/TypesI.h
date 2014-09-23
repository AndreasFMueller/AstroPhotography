/*
 * TypesI.h -- conversion functions for types
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _TypesI_h
#define _TypesI_h

#include <types.h>
#include <AstroTypes.h>
#include <AstroImage.h>
#include <AstroCoordinates.h>

namespace snowstar {

ImagePoint	convert(const astro::image::ImagePoint& point);
astro::image::ImagePoint	convert(const ImagePoint& point);

ImageSize	convert(const astro::image::ImageSize& size);
astro::image::ImageSize	convert(const ImageSize& size);

ImageRectangle	convert(const astro::image::ImageRectangle& rectangle);
astro::image::ImageRectangle	convert(const ImageRectangle& rectangle);

Point	convert(const astro::Point& point);
astro::Point	convert(const Point& point);

RaDec	convert(const astro::RaDec& radec);
astro::RaDec	convert(const RaDec& radec);

AzmAlt	convert(const astro::AzmAlt& azmalt);
astro::AzmAlt	convert(const AzmAlt& azmalt);

} // namespace snowstar

#endif /* _TypesI_h */
