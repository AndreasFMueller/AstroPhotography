/*
 * Conversions.h -- conversion functions between different classes
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil 
 */
#ifndef _Conversions_h
#define _Conversions_h

#include <device.hh>
#include <AstroTypes.h>
#include <AstroLocator.h>
#include <AstroCamera.h>

namespace astro {

// Device type
Astro::DeviceLocator::device_type convert(
			astro::device::DeviceLocator::device_type fromtype);
astro::device::DeviceLocator::device_type convert(
			Astro::DeviceLocator::device_type fromtype);

// Exposure state
Astro::ExposureState	convert(astro::camera::Exposure::State fromstate);
astro::camera::Exposure::State	convert(Astro::ExposureState fromstate);

// ImagePoint
Astro::ImagePoint	convert(const astro::image::ImagePoint& point);
astro::image::ImagePoint	convert(const Astro::ImagePoint& point);

// ImageSize
Astro::ImageSize	convert(const astro::image::ImageSize& size);
astro::image::ImageSize	convert(const Astro::ImageSize& size);

// ImageRectangle
Astro::ImageRectangle	convert(const astro::image::ImageRectangle& rectangle);
astro::image::ImageRectangle	convert(const Astro::ImageRectangle& rectangle);

// shutter state
astro::camera::shutter_state	convert(const Astro::ShutterState& state);
Astro::ShutterState	convert(const astro::camera::shutter_state state);

// Binning mode
astro::camera::Binning	convert(const Astro::BinningMode& mode);
Astro::BinningMode	convert(const astro::camera::Binning& mode);

astro::camera::BinningSet	convert(const Astro::BinningSet_var set);
Astro::BinningSet_var	convert(const astro::camera::BinningSet& set);

// Exposure
Astro::Exposure	convert(const astro::camera::Exposure& exposure);
astro::camera::Exposure	convert(const Astro::Exposure& exposure);

// relay bits
uint8_t	convert_octet2relaybits(CORBA::Octet bits);
CORBA::Octet	convert_relaybits2octet(uint8_t bits);

} // namespace astro

#endif /* _Conversions_h */
