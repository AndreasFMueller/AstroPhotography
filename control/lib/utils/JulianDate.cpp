/*
 * JulianDate.cpp -- Julian date for coordinate computations
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroCoordinates.h>

namespace astro {

/**
 * \brief Set up the _T variable from the time provided
 *
 * This method does not take the gregorian calendar reform into account,
 * it uses the algorithm described on the wikipedia page
 * https://de.wikipedia.org/wiki/Julianisches_Datum
 */
void	JulianDate::update(time_t when) {
	struct tm	result;
	gmtime_r(&when, &result);

	int	month = result.tm_mon + 1;
	int	year = result.tm_year + 1900;
	int	day = result.tm_mday;

	_H = result.tm_hour / 24. + result.tm_min / 1440.
			+ result.tm_sec / 86400.;

	if (month <= 2) {
		year = year - 1;
		month = month + 12;
	}

	int	A = year / 100;
	int	B = 2 - A + A / 4;

	_T = trunc(365.25 * (year + 4716)) + trunc(30.6001 * (month + 1))
		+ day + _H + B - 1524.5;
	char	buffer[30];
	strftime(buffer, sizeof(buffer), "%F %D", &result);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s is JD=%.3f", buffer, _T);
}

/**
 * \brief update to the current time
 */
void	JulianDate::update() {
	time_t	now;
	time(&now);
	this->update(now);
}

/**
 * \brief Construct Julian Date for current point in time
 */
JulianDate::JulianDate() {
	time_t	now;
	time(&now);
	update(now);
}

/**
 * \brief Construct Julian Date for a given point in time
 */
JulianDate::JulianDate(time_t when) {
	update(when);
}

/**
 * \brief Compute siderial time at greenwich
 *
 * This method is based on the formulas in
 * https://www.cv.nrao.edu/~rfisher/Ephemerides/times.html
 */
Angle	JulianDate::GMST() const {
	// We are using UTC instead of UT1
	// first compute the julian date at midnight
	double	t = trunc(_T + 0.5) - 0.5;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "julian date at midnight: %.1f", t);
	double	T = (t - 2451545.0) / 36525;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "julian centuries: %.8f", T);
	double	g = 24110.54841 + T * (8640184.812866 + T * (0.093104 - 0.0000062 * T));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "GMST at midnight: %f", g);
	double	s = (1.00273790935 + 5.9e-11 * T) * _H * 86400;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "time of day: %.4f", s);
	Angle	result(((g + s) / 86400) * (2 * M_PI));
	result = result.reduced();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "GMST: %s", result.hms().c_str());
	return result;
}

/**
 * \brief Get the number of julian centuries
 */
double	JulianDate::years() const {
	return (_T - 2451545.0) / 36525;
}

} // namespace astro
