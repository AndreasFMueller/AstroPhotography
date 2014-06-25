/*
 * GuiderIconversions.h -- guider factory interface declarations
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _GuiderIconversions_h
#define _GuiderIconversions_h

#include <guider.h>
#include <AstroGuiding.h>

namespace snowstar {

GuiderDescriptor	convert(const astro::guiding::GuiderDescriptor& gd);
astro::guiding::GuiderDescriptor	convert(const GuiderDescriptor& gd);

TrackingPoint	convert(const astro::guiding::TrackingPoint& trackingpoint);

CalibrationPoint	convert(const astro::guiding::CalibrationPoint& cp);

} // namespace snowstar

#endif /* _GuiderIconversions_h */
