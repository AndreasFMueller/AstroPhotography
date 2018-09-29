/*
 * AzimuthConverter.cpp -- convert RA/DEC into Azimuth and Altitude
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroCoordinates.h>

namespace astro {

AzmAltConverter::AzmAltConverter(time_t when, const LongLat& longlat) 
	: JulianDate(when), _longlat(longlat) { 
	_lmst = GMST() - _longlat.longitude();
}
AzmAltConverter::AzmAltConverter(const LongLat& longlat)
	: JulianDate(), _longlat(longlat) {
	_lmst = GMST() - _longlat.longitude();
}

AzmAlt	AzmAltConverter::operator()(const RaDec& radec) {
	Angle	hourangle = _lmst - radec.ra();
	AzmAlt	result;
	double	sh = sin(_longlat.latitude()) * sin(radec.dec())
		+ cos(_longlat.latitude()) * cos(radec.dec()) * cos(hourangle);
	result.alt().radians(asin(sh));
	double	y = sin(hourangle);
	double	x = cos(hourangle) * sin(_longlat.latitude())
			- tan(radec.dec()) * cos(_longlat.latitude());
	result.azm().radians(atan2(y, x));
	return result;
}

} // namespace astro
