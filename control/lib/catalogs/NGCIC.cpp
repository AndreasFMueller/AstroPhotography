/*
 * NGCIC.cpp -- Access to the NGC/IC catalog
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "NGCIC.h"
#include "MappedFile.h"
#include <AstroDebug.h>
#include <AstroFormat.h>

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
		result.mag() = std::stod(record.substr(40, 4));
	} catch (...) {
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "no magnitude for %s",
		//	result.name.c_str());
	}

	// constellation
	result.constellation = record.substr(29, 3);

	// classification
	std::string	key = record.substr(6, 3);
	if (key == "Gx ") {
		result.classification = DeepSkyObject::Galaxy;
	}
	if (key == "OC ") {
		result.classification = DeepSkyObject::OpenCluster;
	}
	if (key == "Gb ") {
		result.classification = DeepSkyObject::GlobularCluster;
	}
	if (key == "Nb ") {
		result.classification = DeepSkyObject::BrightNebula;
	}
	if (key == "Pl ") {
		result.classification = DeepSkyObject::PlanetaryNebula;
	}
	if (key == "C+N") {
		result.classification = DeepSkyObject::ClusterNebulosity;
	}
	if (key == "Ast") {
		result.classification = DeepSkyObject::Asterism;
	}
	if (key == "Kt ") {
		result.classification = DeepSkyObject::Knot;
	}
	if (key == "***") {
		result.classification = DeepSkyObject::TripleStar;
	}
	if (key == "D* ") {
		result.classification = DeepSkyObject::DoubleStar;
	}
	if (key == "*  ") {
		result.classification = DeepSkyObject::SingleStar;
	}
	if (key == "?  ") {
		result.classification = DeepSkyObject::Uncertain;
	}
	if (key == "   ") {
		result.classification = DeepSkyObject::Unidentified;
	}
	if (key == "-  ") {
		result.classification = DeepSkyObject::Nonexistent;
	}
	if (key == "PD ") {
		result.classification = DeepSkyObject::PlateDefect;
	}

	// size
	try {
		result.size.degrees(std::stod(record.substr(33, 5)) / 60);
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
NGCIC::NGCIC(const std::string& filename) {
	// map the file into the address space
	MappedFile	ngcfile(filename, 97);

	// now iterate through all the records
	for (size_t recno = 0; recno < ngcfile.nrecords(); recno++) {
		std::string	record = ngcfile.get(recno);
		DeepSkyObject	object = Object_from_Record(record);
		insert(std::make_pair(object.name, object));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%u objects in catalog", size());
}

/**
 * \brief retrieve a single object by name
 */
DeepSkyObject	NGCIC::find(const std::string& name) const {
	const_iterator	o = std::map<std::string, DeepSkyObject>::find(name);
	if (o == end()) {
		throw std::runtime_error("object " + name + " not found");
	}
	return o->second;
}

/**
 * \brief Retrieve all objects in a RA/DEC rectangle
 */
NGCIC::objectsetptr	NGCIC::find(const SkyWindow& window) const {
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

} // namespace catalog
} // namespace astro
