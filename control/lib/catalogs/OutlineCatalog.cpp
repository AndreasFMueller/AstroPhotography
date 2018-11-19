/*
 * OutlineCatalog.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroCatalog.h>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <fstream>
#include <unistd.h>

namespace astro {
namespace catalog {

/**
 * \brief Auxiliary function to remove blanks
 */
static std::string	removeblanks(const std::string& v) {
	std::string	result;
	std::copy_if(v.begin(), v.end(), std::back_inserter(result),
		[&](char c) -> bool {
			return c != ' ';
		}
	);
	return result;
}

/**
 * \brief Parse the data file
 */
void	OutlineCatalog::parse(const std::string& directory) {
	// construct the file name
	std::string	filename(directory + "/outlines.data");

	// make sure the file exists by opening it for reading
	std::ifstream	in(filename.c_str());
	if (!in.good()) {
		std::string	msg = stringprintf("cannot get %s",
			filename.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// read through the catalog line by line
	Outline	outline("");
	while (!in.eof()) {
		char	buffer[1024];
		double	ra, dec;
		in >> ra;
		in >> dec;
		std::string	cmd;
		in >> cmd;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "ra=%f, dec=%f, cmd='%s'",
			ra, dec, cmd.c_str());

		// the start command starts a new outline
		if (cmd == "start") {
			// clear the string
			outline.clear();

			// read the name to the end of the line
			in.getline(buffer, sizeof(buffer));
			std::string	n = trim(std::string(buffer));
			n = removeblanks(n);
			outline.name(n);
		}

		// all lines contain point data
		RaDec	radec;
		radec.ra() = Angle(ra, Angle::Hours);
		radec.dec() = Angle(dec, Angle::Degrees);
		outline.push_back(radec);

		if (cmd == "vertex") {
			// no special processing needed
		}

		// the end command defines the end of an outline definition
		if (cmd == "end") {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "inserting object '%s'",
				outline.toString().c_str());
			_outlinemap.insert(std::make_pair(outline.name(),
				outline));
		}

	}

	// report the number of outlines found
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d outlines found",
		_outlinemap.size());
	
	// close the file
	in.close();
}

/**
 * \brief Default constructor from standard star catalog location
 */
OutlineCatalog::OutlineCatalog() {
	std::string	pathbase(DATAROOTDIR "/starcatalogs/stellarium");
	parse(pathbase);
}

/**
 * \brief Construction from a base directory name
 */
OutlineCatalog::OutlineCatalog(const std::string& directory) {
	parse(directory);
}

/**
 * \brief Find out whether an object has an outline
 *
 * \param name	name of the object
 */
bool	OutlineCatalog::has(const std::string& name) const {
	return _outlinemap.find(name) != _outlinemap.end();
}

/**
 * \brief Find the object in the database
 *
 * This method throws an exception if the object is not found
 *
 * \param name	name of the object
 */
Outline	OutlineCatalog::find(const std::string& name) const {
	auto i = _outlinemap.find(name);
	if (i != _outlinemap.end()) {
		return i->second;
	}
	std::string	msg = stringprintf("no outline for '%s' found",
		name.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

} // namespace catalog
} // namespace astro
