/*
 * Horizon.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroHorizon.h>
#include <AstroDebug.h>
#include <AstroConfig.h>
#include <iostream>
#include <fstream>
#include <string>

namespace astro {
namespace horizon {

/**
 * \brief Add a base point with azimuth 0
 */
void	Horizon::addbasepoint() {
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
	if (in.fail()) {
		std::string	msg = stringprintf("cannot open file %s",
			csvfilename.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// read the file
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start reading file %s",
		csvfilename.c_str());
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
			std::string	azifield;
			std::string	altfield;

			if (l.size() > 2) {
				azifield = l[7];
				altfield = l[10];
			} else {
				azifield = l[0];
				altfield = l[1];
			}
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

	addbasepoint();
}

/**
 * \brief Construct a rotated horizon
 *
 * \param other		the horizon to rotate
 * \param angle		the rotation angle
 */
Horizon::Horizon(const Horizon& other, const Angle& angle) {
	for (auto& i : other) {
		AzmAlt	point((i.azm() + angle).reduced(), i.alt());
		insert(point);
	}
	addbasepoint();
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

/**
 * \brief Get a new rotated 
 *
 * \param angle		the rotation angle
 */
HorizonPtr	Horizon::rotate(const Angle& angle) const {
	return HorizonPtr(new Horizon(*this, angle));
}

static HorizonPtr	default_horizon;

// horizon file name
static config::ConfigurationKey		_horizon_file_name_key(
		"gui", "horizon", "filename");
static config::ConfigurationRegister    _horizon_file_name_registration(
	_horizon_file_name_key, 
	"file name of the horizon file to use in the sky display");

// rotation key
static config::ConfigurationKey		_horizon_rotate_key(
		"gui", "horizon", "rotate");
static config::ConfigurationRegister    _horizon_rotate_registration(
	_horizon_rotate_key,
	"angle in degrees the horizon file needs to be rotated");


/**
 * \brief Get the default horizon
 *
 * This method expects the horizon to be found in ~/.astro/horizon.csv
 */
HorizonPtr	Horizon::get() {
	if (default_horizon) {
		return default_horizon;
	}
	Angle	rotationangle;

	// check the default configuration for a rotation angle
	config::ConfigurationPtr	config = config::Configuration::get();
	if (config->has(_horizon_rotate_key)) {
		std::string anglestring = config->get(_horizon_rotate_key);
		rotationangle = Angle(std::stod(anglestring), Angle::Degrees);
	}

	// first check whether we have a configuration
	std::string	filename;
	if (config->has(_horizon_file_name_key)) {
		filename = config->get(_horizon_file_name_key);
	} else {
		// check the home directory
		char	*home = getenv("HOME");
		if (NULL == home) {
			std::string	msg = stringprintf("HOME not set");
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
		filename = stringprintf("%s/.astro/horizon.csv", home);
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "try %s as horizon file",
		filename.c_str());

	// remember this horizon object
	default_horizon = get(filename);
	return default_horizon;
}

/**
 * \brief Get the horizon of the file
 *
 * \param filename	the name of the file
 */
HorizonPtr	Horizon::get(const std::string& filename) {
	// create the new Horizon object
	Horizon	*horizonp = NULL;
	try {
		horizonp = new Horizon(filename);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "can't create horizon from %s: %s",
			filename.c_str(), x.what());
		throw x;
	}
	return HorizonPtr(horizonp);
}

/**
 * \brief Construct a rotated horizon 
 *
 * \param filename	the file name of the horizon file
 * \param angle		the rotation angle
 */
HorizonPtr	Horizon::get(const std::string& filename, const Angle& angle) {
	HorizonPtr	horizon = get(filename);
	return horizon->rotate(angle);
}

/**
 * \brief Interpolate points where the angle changes sign
 */
void	Horizon::flatten() {
	// got through all points and add an intermediate point whenever
	// a point is below flat the horizon
	auto	i = begin();
	AzmAlt	previous = *i;
	while (++i != end()) {
		AzmAlt	next = *i;

		// is it necessary to add an intermediate point?
		if (((previous.a2() < 0) && (next.a2() > 0)) || 
		 	((previous.a2() > 0) && (next.a2() < 0))) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"interpolate between %s and %s",
				previous.toString().c_str(),
				next.toString().c_str());
			double	x1 = previous.a1().radians();
			double	x2 = next.a1().radians();
			double	y1 = fabs(previous.a2().radians());
			double	y2 = fabs(next.a2().radians());
			double	l = x2 - x1;
			double	x = x1 + l * y1 / (y1 + y2);
			AzmAlt	intermediate(Angle(x), 0);
			insert(intermediate);
		}

		previous = next;
	}

	// remove all entries with negative 
	for (i = begin(); i != end(); i++) {
		if (i->a2() < 0) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "remove %s",
				i->toString().c_str());
			erase(i);
		}
	}
}

} // namespace horizon
} // namespace astro
