/*
 * CcdIconversions.h -- conversions needed for the ccdi interface
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _CcdIconversions_h
#define _CcdIconversions_h

#include <camera.h>
#include <AstroCamera.h>

namespace snowstar {

CcdInfo	convert(const astro::camera::CcdInfo& info);

Exposure	convert(const astro::camera::Exposure& exp);
astro::camera::Exposure	convert(const Exposure& exp);

ExposureState	convert(const astro::camera::Exposure::State& state);
astro::camera::Exposure::State	convert(const ExposureState& state);

ShutterState	convert(const astro::camera::shutter_state& state);
astro::camera::shutter_state	convert(const ShutterState& state);

} // namespace snowstar

#endif /* _CcdIconversions_h */
