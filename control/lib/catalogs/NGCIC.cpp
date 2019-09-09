/*
 * NGCIC.cpp -- Access to the NGC/IC catalog
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "NGCIC.h"
#include "MappedFile.h"
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <fstream>

namespace astro {
namespace catalog {

static DeepSkyObject	Object_from_Record(const std::string& record) {
	DeepSkyObject	result;

	// name
	bool	IC = (record[0] == 'I');
	int	number = std::stoi(record.substr(1, 4));
	result.name = stringprintf("%s%d", (IC) ? "IC" : "NGC", number);

	// position
	result.ra().hours(
		std::stoi(record.substr(10, 2))
		+ std::stod(record.substr(13, 4)) / 60
	);
	result.dec().degrees(
		((record[19] == '-') ? (-1) : 1)
		* (std::stoi(record.substr(20, 2))
		+ std::stoi(record.substr(23, 2)) / 60.)
	);

	// magnitude
	try {
		result.mag(std::stod(record.substr(40, 4)));
	} catch (...) {
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "no magnitude for %s",
		//	result.name.c_str());
	}

	// constellation
	result.constellation = record.substr(29, 3);

	// classification
	std::string	key = record.substr(6, 3);
	if (key == " Gx") {
		result.classification = DeepSkyObject::Galaxy;
	}
	if (key == " OC") {
		result.classification = DeepSkyObject::OpenCluster;
	}
	if (key == " Gb") {
		result.classification = DeepSkyObject::GlobularCluster;
	}
	if (key == " Nb") {
		result.classification = DeepSkyObject::BrightNebula;
	}
	if (key == " Pl") {
		result.classification = DeepSkyObject::PlanetaryNebula;
	}
	if (key == "C+N") {
		result.classification = DeepSkyObject::ClusterNebulosity;
	}
	if (key == "Ast") {
		result.classification = DeepSkyObject::Asterism;
	}
	if (key == " Kt") {
		result.classification = DeepSkyObject::Knot;
	}
	if (key == "***") {
		result.classification = DeepSkyObject::TripleStar;
	}
	if (key == " D*") {
		result.classification = DeepSkyObject::DoubleStar;
	}
	if (key == "  *") {
		result.classification = DeepSkyObject::SingleStar;
	}
	if (key == "  ?") {
		result.classification = DeepSkyObject::Uncertain;
	}
	if (key == "   ") {
		result.classification = DeepSkyObject::Unidentified;
	}
	if (key == "  -") {
		result.classification = DeepSkyObject::Nonexistent;
	}
	if (key == " PD") {
		result.classification = DeepSkyObject::PlateDefect;
	}

	// size
	try {
		Angle	s(std::stod(record.substr(33, 5)) / 60, Angle::Degrees);
		result.axes(TwoAngles(s, s));
	} catch (...) {
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "no size for %s",
		//	result.name.c_str());
	}

	// return the result
	return result;
}

/**
 * \brief Construct the catalog from the file
 */
NGCIC::NGCIC(const std::string& dirname) {
	// map the file into the address space
	std::string	ngcfilename(dirname + "/ngc2000.dat");
	MappedFile	ngcfile(ngcfilename, 97);

	// now iterate through all the records
	for (size_t recno = 0; recno < ngcfile.nrecords(); recno++) {
		std::string	record = ngcfile.get(recno);
		DeepSkyObject	object = Object_from_Record(record);
		insert(std::make_pair(object.name, object));
		names.insert(std::make_pair(object.name, object.name));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%u objects in catalog", size());

	// Open the names file to build the names map
	std::string	namesfilename(dirname + "/names.dat");
	std::ifstream	in(namesfilename.c_str());
	if (in.fail()) {
		std::string	msg = stringprintf("names file '%s' missing",
			namesfilename.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// read the stream line by line
	do {
		char	buffer[1024];
		in.getline(buffer, sizeof(buffer));
		int	number = std::stoi(std::string(buffer + 37, 4));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "number: %d", number);
		std::string	name;
		if (buffer[36] == 'I') {
			name = stringprintf("IC%d", number);
		} else {
			name = stringprintf("NGC%d", number);
		}

		// name 
		std::string	n;
		if ((buffer[0] == 'M') && (buffer[1] == ' ')) {
			// Messier names
			n = std::string("M") + trim(std::string(buffer + 1, 4));
		} else {
			n = trim(std::string(buffer, 36));
		}

		debug(LOG_DEBUG, DEBUG_LOG, 0, "installing '%s' -> '%s'",
			n.c_str(), name.c_str());
		names.insert(std::make_pair(n, name));
	} while (!in.eof());
}

/**
 * \brief retrieve a single object by name
 */
DeepSkyObject	NGCIC::find(const std::string& name) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "searching NGC/IC for '%s' %d",
		name.c_str(), name.size());
	// handle special M names that are missing from NGC
	if (name == std::string("M24")) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "M24 found");
		DeepSkyObject	object;
		object.name = name;
		object.constellation = std::string("Sagittarius");
		object.mag(4.6);
		object.ra() = Angle(18.28, Angle::Hours);
		object.dec() = Angle(-18.55, Angle::Degrees);
		object.classification = DeepSkyObject::OpenCluster;
		return object;
	} else if (name == std::string("M40")) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "M40 found");
		DeepSkyObject	object;
		object.name = name;
		object.constellation = std::string("Ursa Maior");
		object.mag(9.7);
		object.ra() = Angle(12.37015, Angle::Hours);
		object.dec() = Angle(58.08294, Angle::Degrees);
		object.classification = DeepSkyObject::DoubleStar;
		return object;
	} else if (name == std::string("M45")) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "M45 found");
		DeepSkyObject	object;
		object.name = name;
		object.constellation = std::string("Taurus");
		object.mag(1.6);
		object.ra() = Angle(3.79, Angle::Hours);
		object.dec() = Angle(24.11667, Angle::Degrees);
		object.classification = DeepSkyObject::OpenCluster;
		return object;
	} else if (name == std::string("M103")) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "M103 found");
		DeepSkyObject	object;
		object.name = name;
		object.constellation = std::string("Cassiopeia");
		object.mag(7.4);
		object.ra() = Angle(1.55606, Angle::Hours);
		object.dec() = Angle(60.65806, Angle::Degrees);
		object.classification = DeepSkyObject::OpenCluster;
		return object;
	}

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

	// name search
	auto	i = names.find(name);
	if (i != names.end()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "name %s found: '%s'",
			name.c_str(), i->second.c_str());
		return find(i->second);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "name %s not found",
			name.c_str());
	}

	// 
	throw std::runtime_error("object " + name + " not found");
}

/**
 * \brief Retrieve all objects in a RA/DEC rectangle
 */
DeepSkyObjectSetPtr	NGCIC::find(const SkyWindow& window) const {
	DeepSkyObjectSet	*result = new DeepSkyObjectSet();
	DeepSkyObjectSetPtr	resultptr(result);
	std::map<std::string, DeepSkyObject>::const_iterator	o;
	for (o = begin(); o != end(); o++) {
		if (window.contains(o->second)) {
			result->insert(o->second);
		}
	}
	return resultptr;
}

std::set<std::string>	NGCIC::findLike(const std::string& name) const {
	std::set<std::string>	result;
	size_t	l = name.size();
	// find matching names
	for (auto i = names.begin(); i != names.end(); i++) {
		if (i->first.size() < l)
			continue;
		if ((i->first.size() == l) && (i->first == name)) {
			result.insert(i->first);
			continue;
		}
		if (i->first.substr(0, l) == name) {
			result.insert(i->first);
		}
	}
	// find matching NGC/IC names
	for (auto i = begin(); i != end(); i++) {
		if (i->first.size() < l)
			continue;
		if ((i->first.size() == l) && (i->first == name)) {
			result.insert(i->first);
			continue;
		}
		if (i->first.substr(0, l) == name) {
			result.insert(i->first);
		}
	}
	return result;
}

} // namespace catalog
} // namespace astro
