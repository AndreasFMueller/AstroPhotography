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
void	OutlineCatalog::parseOutlines(const std::string& directory) {
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
 * \brief Split catalog
 */
static std::vector<std::string>	splitline(const char *data) {
	std::vector<std::string>	result;
	int	i = 0;
	int	j = 0;
	do {
		j++;
		if ((data[j] == '\t') || (data[j] == '\n')) {
			result.push_back(std::string(data + i, j - i));
			i = j + 1;
		}
	} while (data[j] != '\n');
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d components", result.size());
	return result;
}

/**
 * \brief Parse the main catalog and extract ellipses
 */
void	OutlineCatalog::parseEllipses(const std::string& directory) {
	// construct the file name
	std::string	filename(directory + "/catalog.txt");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "parsing %s", filename.c_str());

	// open the input file
	std::ifstream	in(filename.c_str());
	if (in.fail()) {
		std::string	msg = stringprintf("catalog '%s' missing",
			filename.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	while (!in.eof()) {
		char	buffer[1024];
		in.getline(buffer, sizeof(buffer));

		// skip comment lines
		if (buffer[0] == '#')
			continue;

		// split at tab characters
		std::vector<std::string>	components = splitline(buffer);

		// tell about the object we are going to parse
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "parsing %d",
		//	std::stoi(components[0]));

		// check whether the object has an NGC number and skip if it
		// already has an outline
		int	ngc = std::stoi(components[16]);
		std::string	ngcname = stringprintf("NGC%d", ngc);
		if (ngc > 0) {
			if (_outlinemap.find(ngcname) != _outlinemap.end()) {
				continue;
			}
		}

		// check whether the object has an IC number and skip if it
		// already has an outline
		int	ic = std::stoi(components[17]);
		std::string	icname = stringprintf("IC%d", ic);
		if (ic > 0) {
			if (_outlinemap.find(icname) != _outlinemap.end()) {
				continue;
			}
		}

		// check whether the object has an Messier number and skip if
		// it already has an outline
		int	messier = std::stoi(components[18]);
		std::string	messiername = stringprintf("M%d", messier);
		if (messier > 0) {
			if (_outlinemap.find(messiername) != _outlinemap.end()) {
				continue;
			}
		}

		// skip if in none of those catalogs
		if ((ngc == 0) && (ic == 0) && (messier == 0))
			continue;

		// create an ellipse outline object from the data
		RaDec	position;
		position.ra() = Angle(std::stod(components[1]),
			Angle::Degrees);
		position.dec() = Angle(std::stod(components[2]),
			Angle::Degrees);

		TwoAngles	dimensions;
		dimensions.a1() = Angle(std::stod(components[7]) / 60,
			Angle::Degrees);
		dimensions.a2() = Angle(std::stod(components[8]) / 60,
			Angle::Degrees);

		Angle	orientation(std::stod(components[9]), Angle::Degrees);

		// add outline to the map
		Outline	outline("blubb", position, dimensions, orientation);
		if (ngc > 0) {
			Outline	ngcoutline(ngcname);
			ngcoutline = outline;
			ngcoutline.name(ngcname);
			_outlinemap.insert(std::make_pair(ngcoutline.name(),
				ngcoutline));
		}
		if (ic > 0) {
			Outline	icoutline(icname);
			icoutline = outline;
			icoutline.name(icname);
			_outlinemap.insert(std::make_pair(icoutline.name(),
				icoutline));
		}
		if (messier > 0) {
			Outline	messieroutline(messiername);
			messieroutline = outline;
			messieroutline.name(messiername);
			_outlinemap.insert(std::make_pair(messieroutline.name(),
				messieroutline));
		}
	}

	// close the file
	in.close();
}

/**
 * \brief Parse the stellarium catalog
 */
void	OutlineCatalog::parse(const std::string& directory) {
	parseOutlines(directory);
	parseEllipses(directory);
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
