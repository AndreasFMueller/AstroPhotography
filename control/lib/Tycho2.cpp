/*
 * Tycho2.cpp -- Tycho2 star catalog class declarations
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Tycho2.h>
#include <includes.h>
#include <AstroFormat.h>
#include <AstroDebug.h>

namespace astro {
namespace catalog {

#define TYCHO2_RECORD_LENGTH	207

//////////////////////////////////////////////////////////////////////
// Tycho2 star class implementation
//////////////////////////////////////////////////////////////////////
void	Tycho2Star::setup(const std::string& line) {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "creating star from line '%s'",
	//	line.c_str());
	float	vt = std::stod(line.substr(123, 6));
	float	bt = std::stod(line.substr(110, 6));
	_mag = vt - 0.090 * (bt - vt);
if (_mag < 2) {
debug(LOG_DEBUG, DEBUG_LOG, 0, "%s -> %f", line.substr(123,6).c_str(), _mag);
}
	// RA and DEC
	ra().degrees(std::stod(line.substr(15, 12)));
	dec().degrees(std::stod(line.substr(28, 12)));

	// proper motion
	pm().ra().degrees(std::stod(line.substr(41, 7)) / 3600000);
	pm().dec().degrees(std::stod(line.substr(49, 7)) / 3600000);

	//debug(LOG_DEBUG, DEBUG_LOG, 0, "%s %s %.3f",
	//	ra().hms().c_str(), dec().dms().c_str(),
	//	mag());
}

Tycho2Star::Tycho2Star(const char *line) {
	setup(std::string(line, TYCHO2_RECORD_LENGTH));
}

Tycho2Star::Tycho2Star(const std::string& line) {
	setup(line);
}

//////////////////////////////////////////////////////////////////////
// Tycho2 catalog class implementation
//////////////////////////////////////////////////////////////////////

Tycho2::Tycho2(const std::string& filename) : _filename(filename) {
	// find the file
	struct stat	sb;
	if (stat(filename.c_str(), &sb) < 0) {
		std::string	msg = stringprintf("cannot stat Tycho2 file "
			"%s: %s", _filename.c_str(), strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	data_len = sb.st_size;
	_nstars = data_len / TYCHO2_RECORD_LENGTH;
	if (data_len != (TYCHO2_RECORD_LENGTH * _nstars)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "length does not match");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%u stars in catalog", _nstars);

	// open the file for reading
	int	fd = open(_filename.c_str(), O_RDONLY);
	if (fd < 0) {
		std::string	msg = stringprintf("cannot open '%s': %s",
			_filename.c_str(), strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s");
		throw std::runtime_error(msg);
	}

	// map the file into the address space
	data_ptr = (char *)mmap(NULL, sb.st_size, PROT_READ,
		MAP_FILE | MAP_PRIVATE, fd, 0);
	if ((void *)(-1) == data_ptr) {
		close(fd);
		std::string	msg = stringprintf("cannot map '%s': %s",
			_filename.c_str(), strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// we can now close the file descriptor
	close(fd);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Tycho2 catalog opened");
}

Tycho2::~Tycho2() {
	if (NULL != data_ptr) {
		munmap(data_ptr, data_len);
	}
}

/**
 * \brief get a star from the catalog
 */
Tycho2Star	Tycho2::find(unsigned int index) const {
	if (index >= _nstars) {
		throw std::runtime_error("not that many stars in Tycho2");
	}
	size_t	offset = index * TYCHO2_RECORD_LENGTH;
	return Tycho2Star(&data_ptr[offset]);
}

/**
 * \brief get all stars from a window
 */
std::set<Tycho2Star>	Tycho2::find(const SkyWindow& window,
				double minimum_magnitude) {
	std::set<Tycho2Star>	result;
	for (unsigned int index = 0; index < _nstars; index++) {
		try {
			Tycho2Star	star = find(index);
			if ((window.contains(star))
				&& (star.mag() < minimum_magnitude)) {
				result.insert(star);
			}
		} catch (std::exception& x) {
/*
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot get star %u: %s",
				index, x.what());
*/
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %u stars", result.size());
	return result;
}

} // namespace catalog
} // namespace astro
