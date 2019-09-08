/*
 * MilkyWay.cpp
 *
 * (c) 2019 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroCatalog.h>
#include <AstroDebug.h>
#include "json.hpp"
#include <fstream>

using json = nlohmann::json;

namespace astro {
namespace catalog {

std::string	MilkyWay::default_path(DATAROOTDIR
			"/starcatalogs/d3-celestial/mw.json");

/**
 * \brief Construct the MilkyWay from a stream
 */
MilkyWay::MilkyWay(std::istream& in) {
	parse(in);
}

/**
 * \brief Construct the MilkyWay from a file 
 */
MilkyWay::MilkyWay(const std::string& filename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "opening file %s", filename.c_str());
	std::ifstream	in(filename);
	parse(in);
}

/**
 * \brief Construct a MilkyWay from the default path
 */
MilkyWay::MilkyWay() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "opening file %s", default_path.c_str());
	std::string	filename(default_path);
	std::ifstream	in(filename);
	parse(in);
}

/**
 * \brief Parse a stream as a JSON file and then interpret it as outlines
 *
 * \param in	stream about 
 */
void	MilkyWay::parse(std::istream& in) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "parse milkyway jason file");
	// read the milkyway data
	json	j;
	in >> j;

	// access the polygons
	json	features = j["features"];
	int	level = 0;
	for (json::iterator fit = features.begin(); fit != features.end();
		fit++) {
		// create a new outlinelist
		OutlineListPtr	outlinelist(new OutlineList());
		insert(std::make_pair(level, outlinelist));
		int	counter = 0;
		json	jj = (*fit)["geometry"]["coordinates"][0];
		for (json::iterator it = jj.begin(); it != jj.end(); it++) {
			// create a new outline
			Outline	*outline = new Outline(
				astro::stringprintf("o%d", counter++));
			outlinelist->push_back(OutlinePtr(outline));
			// create a new outline
			for (json::iterator pi = it->begin(); pi != it->end();
				pi++) {
				Angle	ra((*pi)[0], Angle::Degrees);
				Angle	dec((*pi)[1], Angle::Degrees);
				outline->push_back(RaDec(ra, dec));
			}
			debug(LOG_DEBUG, DEBUG_LOG, 0, "outline %s: %d points",
				outline->name().c_str(), outline->size());
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "level %d: %d outlines", level,
			outlinelist->size());
		level++;
	}

	//std::cout << j.dump(4) << std::endl;
}

/**
 * \brief Factory method for default file
 */
MilkyWayPtr	MilkyWay::get() {
	MilkyWayPtr	result(new MilkyWay());
	return result;
}

/**
 * \brief Factory method from arbitrary path
 */
MilkyWayPtr	MilkyWay::get(const std::string& filename) {
	MilkyWayPtr	result(new MilkyWay(filename));
	return result;
}

} // namespace catalog
} // namespace astro
