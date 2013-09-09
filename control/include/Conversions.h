/*
 * Conversions.h -- conversion functions between different classes
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil 
 */
#ifndef _Conversions_h
#define _Conversions_h

#include <module.hh>
#include <AstroTypes.h>
#include <AstroLocator.h>
#include <AstroCamera.h>

namespace astro {

// Device type
astro::device::DeviceLocator::device_type convert(
			Astro::DeviceLocator::device_type fromtype);
Astro::DeviceLocator::device_type convert(
			astro::device::DeviceLocator::device_type fromtype);
std::string	convert2string(Astro::DeviceLocator::device_type fromtype);
std::string	convert2string(
			astro::device::DeviceLocator::device_type fromtype);

// Exposure state
astro::camera::Exposure::State	convert(Astro::ExposureState fromstate);
Astro::ExposureState	convert(astro::camera::Exposure::State fromstate);
std::string	convert2string(Astro::ExposureState fromstate);
std::string	convert2string(astro::camera::Exposure::State fromstate);

// ImagePoint
astro::image::ImagePoint	convert(const Astro::ImagePoint& point);
Astro::ImagePoint	convert(const astro::image::ImagePoint& point);

// ImageSize
astro::image::ImageSize	convert(const Astro::ImageSize& size);
Astro::ImageSize	convert(const astro::image::ImageSize& size);

// ImageRectangle
astro::image::ImageRectangle	convert(const Astro::ImageRectangle& rectangle);
Astro::ImageRectangle	convert(const astro::image::ImageRectangle& rectangle);

// shutter state
astro::camera::shutter_state	convert(const Astro::ShutterState& state);
Astro::ShutterState	convert(const astro::camera::shutter_state state);

// Binning mode
astro::camera::Binning	convert(const Astro::BinningMode& mode);
Astro::BinningMode	convert(const astro::camera::Binning& mode);

// BinningSet
astro::camera::BinningSet	convert(const Astro::BinningSet_var set);
astro::camera::BinningSet	convert(const Astro::BinningSet set);
Astro::BinningSet_var	convert(const astro::camera::BinningSet& set);

// Exposure
astro::camera::Exposure	convert(const Astro::Exposure& exposure);
Astro::Exposure	convert(const astro::camera::Exposure& exposure);

// relay bits
uint8_t	convert_octet2relaybits(CORBA::Octet bits);
CORBA::Octet	convert_relaybits2octet(uint8_t bits);

// CcdInfo
astro::camera::CcdInfo	convert(const Astro::CcdInfo& info);
astro::camera::CcdInfo	convert(const Astro::CcdInfo_var& info);
Astro::CcdInfo	convert(const astro::camera::CcdInfo& info);

} // namespace astro

#endif /* _Conversions_h */
