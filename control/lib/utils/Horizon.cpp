/*
 * Horizon.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroHorizon.h>
#include <AstroDebug.h>
#include <iostream>
#include <fstream>
#include <string>

namespace astro {
namespace horizon {

/**
 * \brief Construct a null horizon
 */
Horizon::Horizon() {
	AzmAlt	point(Angle(0.), Angle(0.));
	insert(point);
}

/**
 * \brief Construct a horizon with a given altitude
 *
 * \param alt	the altitude to use all around
 */
Horizon::Horizon(const Angle& alt) {
	AzmAlt	point(Angle(0.), alt);
	insert(point);
}

/**
 * \brief Construct a horizon from a CSV file
 *
 * \param csvfilename	name of the data file
 */
Horizon::Horizon(const std::string& csvfilename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "parsing csv file %s",
		csvfilename.c_str());
	// open the file for reading
	std::ifstream	in(csvfilename.c_str());
	while (!in.eof()) {
		char	buf[1024];
		// read the file line by line (skipping the first line)
		in.getline(buf, sizeof(buf));

		// try to split the line
		if (!in.fail()) {
			std::string	line(buf);
			// split a line into fields
			std::vector<std::string>	l;
			split<std::vector<std::string> >(line, ",", l);

			// extract the fields
			std::string	azifield = l[7];
			std::string	altfield = l[10];
			debug(LOG_DEBUG, DEBUG_LOG, 0, "azi = %s, alt = %s",
				azifield.c_str(), altfield.c_str());
			try {
				// convert the fields into angles
				Angle	azi(std::stod(azifield),
						Angle::Degrees);
				Angle	alt(std::stod(altfield),
						Angle::Degrees);

				// convert to a point
				AzmAlt	point(azi, alt);
				debug(LOG_DEBUG, DEBUG_LOG, 0, "got %s", 
					point.toString().c_str());
				insert(point);
			} catch (const std::exception& x) {
				debug(LOG_ERR, DEBUG_LOG, 0,
					"cannot convert: %s", x.what());
			}
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s has %d points", csvfilename.c_str(),
		size());

	// add null horizon of there are no points in the file
	if (size() == 0) {
		AzmAlt	point(Angle(0.), Angle(0.));
		insert(point);
		return;
	}

	// find out whether adding a base point is necessary
	AzmAlt	first = *begin();
	if (first.azm() == Angle(0.)) {
		return;
	}

	// build the base point
	AzmAlt	last = *rbegin();
	double	u = first.azm().degrees();
	double	v = 360 - last.azm().degrees();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "weights: u = %f, v = %f", u, v);
	Angle	alt = (last.alt() * u + first.alt() * v) * (1 / (u + v));
	insert(AzmAlt(Angle(0.), alt));
}

/**
 * \brief Interpolate points 
 *
 * \param gridconstant	the grid spacing to use when interpolating
 */
void	Horizon::addgrid(const Angle& gridconstant) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding grid points, constant = %s",
		gridconstant.dms().c_str());
	int	s = trunc(360 / gridconstant.degrees());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "number of steps: %d", s);
	for (int i = 1; i < s; i++) {
		// get the angle for this grid point
		Angle	azm = gridconstant * (double)i;

		// find the neighbours
		AzmAlt	second = *begin();
		AzmAlt	first = second;
		Horizon::const_iterator	a = begin();
		bool	found = false;
		do {
			first = second;
			a++;
			if (a == end()) {
				// we are at the end
				break;
			}
			second = *a;
			found = (first.azm() < azm) && (azm <= second.azm());
		} while (!found);

		// if an interval was found and the azm concides with the
		// azimuth of the right endpoint, then we don't need to
		// add a point
		if (found) {
			if (second.azm() == azm) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"skip %s because of %s",
					azm.dms().c_str(),
					second.toString().c_str());
				continue;
			}
		}

		// build the interpolated point
		double	u = (azm - first.azm()).degrees();
		double	v = (found)	? (second.azm() - azm).degrees()
					: 360 - azm.degrees();
		Angle	secondalt = (found) ? second.alt() : begin()->alt();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "interpolate between %s and %s",
			first.toString().c_str(),
			(found) ? second.toString().c_str()
				: begin()->toString().c_str());
		Angle	alt = (secondalt * u + first.alt() * v) * (1 / (u + v));

		// add the interpolated point
		AzmAlt	interpolated(azm, alt);
		insert(interpolated);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "interpolated point: %s",
			interpolated.toString().c_str());
	}
}

static HorizonPtr	default_horizon;

/**
 * \brief get the default horizon
 *
 * This method expects the horizon to be found in ~/.astro/horizon.csv
 */
HorizonPtr	Horizon::get() {
	if (default_horizon) {
		return default_horizon;
	}
	
	// check the home directory
	char	*home = getenv("HOME");
	if (NULL == home) {
		std::string	msg = stringprintf("HOME not set");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	std::string	filename = stringprintf("%s/.astro/horizon.csv", home);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "try %s as horizon file",
		filename.c_str());

	// create the new Horizon object
	Horizon	*horizonp = NULL;
	try {
		horizonp = new Horizon(filename);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "can't create horizon from %s: %s",
			filename.c_str(), x.what());
		throw x;
	}

	// remember this horizon object
	default_horizon = HorizonPtr(horizonp);
	return default_horizon;
}

} // namespace horizon
} // namespace astro
