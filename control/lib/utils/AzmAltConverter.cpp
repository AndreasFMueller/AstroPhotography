/*
 * AzimuthConverter.cpp -- convert RA/DEC into Azimuth and Altitude
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroCoordinates.h>

namespace astro {

/**
 * \brief Construct a converter for a given time and place on earth
 */
AzmAltConverter::AzmAltConverter(time_t when, const LongLat& longlat) 
	: _longlat(longlat) { 
	update(when);
}

/**
 * \brief Construct a converter for the current time and a given place on earth
 */
AzmAltConverter::AzmAltConverter(const LongLat& longlat)
	: _longlat(longlat) {
	time_t	when;
	time(&when);
	update(when);
}

/**
 * \brief Perform the conversion
 *
 * \param radec	celestial coordinates to convert
 */
AzmAlt	AzmAltConverter::operator()(const RaDec& radec) {
	Angle	hourangle = _lmst - radec.ra();
	AzmAlt	result;
	// compute the altitude
	double	sh = sin(_longlat.latitude()) * sin(radec.dec())
		+ cos(_longlat.latitude()) * cos(radec.dec()) * cos(hourangle);
	result.alt() = arcsin(sh);

	// compute the azimuth
	double	y = sin(hourangle);
	double	x = cos(hourangle) * sin(_longlat.latitude())
			- tan(radec.dec()) * cos(_longlat.latitude());
	result.azm() = arctan2(y, x);

	// return the pair
	return result;
}

/**
 * \brief Update the time of the converter
 *
 * \param when	time for which to perform the conversion
 */
void	AzmAltConverter::update(time_t when) {
	JulianDate::update(when);
	_lmst = GMST() + _longlat.longitude();
}

/**
 * \brief Update the time of the converter to the current time
 */
void	AzmAltConverter::update() {
	JulianDate::update();
}

/**
 * \brief convert azimuth and altitude into right ascension and declination
 */
RaDec	AzmAltConverter::inverse(const AzmAlt& azmalt) {
	// compute the nautic traingle
	Angle	a = Angle::right_angle - azmalt.alt();
	Angle	c = Angle::right_angle - _longlat.latitude();
	Angle	beta = Angle(M_PI) - azmalt.azm();
	double	cosb = cos(c) * cos(a) + sin(c) * sin(a) * cos(beta);
	Angle	b = arccos(cosb);

	// compute the declination
	Angle	dec = Angle::right_angle - b;

	// computing the hour angle is a little more difficult
	double	sinalpha = sin(a) * sin(beta) / sin(b);
	double	cosalpha = (cos(a) - cos(b) * cos(c)) / (sin(b) * sin(c));

	// compute the hour angle
	Angle	hourangle = arctan2(sinalpha, cosalpha);

	// convert hour angle to right ascension
	Angle	ra = _lmst - hourangle;

	// convert the result
	RaDec	result(ra, dec);
	return result;
}

} // namespace astro
