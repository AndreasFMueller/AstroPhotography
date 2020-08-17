/*
 * PGC.cpp -- Access to the catalog of principal galaxies
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "PGC.h"
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <fstream>
#include <cmath>
#include <limits>

namespace astro {
namespace catalog {

static DeepSkyObject	Object_from_Record(const std::string& record) {
	DeepSkyObject	result;

	// name
	int	number = std::stoi(record.substr(3, 7));
	result.name = stringprintf("PGC%07d", number);

	// position
	try {
		result.ra().hours(
			std::stoi(record.substr(12, 2))
			+ std::stoi(record.substr(14, 2)) / 60.
			+ std::stod(record.substr(16, 4)) / 3600
		);
		result.dec().degrees(
			((record[20] == '-') ? (-1) : 1)
			* (std::stoi(record.substr(21, 2))
			+  std::stoi(record.substr(23, 2)) / 60.
			+  std::stod(record.substr(25, 4)) / 3600)
		);
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("no position for %s: %s",
			result.name.c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// magnitude
	//result.mag(0.0); // this is the default anyway

	// constellation
	result.constellation = "unknown";

	// classification
	std::string	key = record.substr(28, 2);
	if (key == "G ") {
		result.classification = DeepSkyObject::Galaxy;
	}
	if (key == "M ") {
		result.classification = DeepSkyObject::MultipleSystem;
	}
	if (key == "GM") {
		result.classification = DeepSkyObject::GalaxyInMultipleSystem;
	}

	// size
	try {
		std::string	majorstring = record.substr(36, 5);
		Angle	major;
		if (majorstring == " 9.99") {
			major = Angle(std::numeric_limits<double>::quiet_NaN(),
					Angle::Radians);
		} else {
			major = Angle(
				pow(10., std::stod(majorstring)) * 0.1 / 60,
				Angle::Degrees);
		}
		Angle	minor;
		std::string	minorstring = record.substr(50, 4);
		if (minorstring == "9.99") {
			minor = Angle(std::numeric_limits<double>::quiet_NaN(),
					Angle::Radians);
		} else {
			minor = Angle(pow(10., -std::stod(record.substr(50, 4)))
					* major.degrees(), Angle::Degrees);
		}
		result.axes(TwoAngles(major, minor));
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("no size for %s: %s",
			result.name.c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// get position angle
	try {
		std::string	pastring = record.substr(63, 4);
		if (pastring == "999.") {
			result.position_angle(
				std::numeric_limits<double>::quiet_NaN());
		} else {
			double	deg = std::stod(pastring);
			result.position_angle(Angle(deg, Angle::Degrees));
		}
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("no pa for %s: %s",
			result.name.c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// get alternative names
	try {
		int	nnames = std::stoi(record.substr(75, 2));
		for (int n = 0; n < nnames; n++) {
			result.addname(trim(record.substr(78 + n * 22, 22)));
		}
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("names for %s: %s",
			result.name.c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// return the result
	return result;
}

/**
 * \brief Construct the catalog from the file
 */
PGC::PGC(const std::string& dirname) {
	// map the file into the address space
	std::string	pgcfilename(dirname + "/pgc.dat");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reading from %s", pgcfilename.c_str());
	std::ifstream	in(pgcfilename.c_str());
	if (in.fail()) {
		std::string	msg = stringprintf("cannot open PGC file %s",
			pgcfilename.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// read the stream line by line
	do {
		char	buffer[1024];
		in.getline(buffer, sizeof(buffer));
		std::string	record(buffer);
		if (record.size() < 77) {
			debug(LOG_ERR, DEBUG_LOG, 0, "short record: '%s'",
				record.c_str());
		}

		// name 
		std::string	name;
		try {
			name = record.substr(0, 10);
			DeepSkyObject	obj = Object_from_Record(record);
			insert(std::make_pair(obj.name, obj));

			// insert all alternative names
			PGC	*pgc = this;
			for_each(obj.names().begin(), obj.names().end(),
				[pgc,obj](const std::string& n) mutable {
					pgc->insert(std::make_pair(n, obj));
				}
			);
		} catch (const std::exception& x) {
			std::string	msg = stringprintf("constructor %s: %s",
				name.c_str(), x.what());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		}
	} while (!in.eof());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructor complete, %d objects",
		size());
}

/**
 * \brief retrieve a single object by name
 */
DeepSkyObject	PGC::find(const std::string& name) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "searching NGC/IC for '%s' %d",
		name.c_str(), name.size());

	// direct search
	const_iterator	o = std::map<std::string, DeepSkyObject>::find(name);
	if (o != end()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "generic %s found: '%s'",
			name.c_str(), o->second.name.c_str());
		return o->second;
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "generic %s not found",
			name.c_str());
	}

	throw std::runtime_error("object " + name + " not found");
}

/**
 * \brief Retrieve all objects in a RA/DEC rectangle
 */
PGC::objectsetptr	PGC::find(const SkyWindow& window) const {
	objectset	*result = new objectset();
	objectsetptr	resultptr(result);
	std::map<std::string, DeepSkyObject>::const_iterator	o;
	for (o = begin(); o != end(); o++) {
		if (window.contains(o->second)) {
			result->insert(o->second);
		}
	}
	return resultptr;
}

std::set<std::string>	PGC::findLike(const std::string& name,
	size_t maxobjects) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start finding like %s", name.c_str());
	std::set<std::string>	result;
	size_t	l = name.size();
	size_t	counter = 0;
	// find matching PGC names
	for (auto i = begin(); i != end(); i++) {
		if (i->first.size() < l)
			continue;
		if ((i->first.size() == l) && (i->first == name)) {
			result.insert(i->first);
			counter++;
			if (counter >= maxobjects) {
				goto complete;
			}
			continue;
		}
		if (i->first.substr(0, l) == name) {
			result.insert(i->first);
			counter++;
			if (counter >= maxobjects) {
				goto complete;
			}
		}
	}
complete:
	debug(LOG_DEBUG, DEBUG_LOG, 0, "search complete, %d objects",
		result.size());
	return result;
}

} // namespace catalog
} // namespace astro
