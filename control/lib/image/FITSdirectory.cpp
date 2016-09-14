/**
 * FITSdirectory.cpp -- directory containing a large number of FITS files
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroImage.h>
#include <AstroIO.h>
#include <AstroFormat.h>
#include <istream>
#include <fstream>
#include <AstroDebug.h>

using namespace astro::image;

namespace astro {
namespace io {

/**
 * \brief Common setup function for all FITSdirectory constructors
 */
void	FITSdirectory::setup() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "format: %s, path: %s",
		(_format == TIMESTAMP) ? "timestamp" : "counter",
		_path.c_str());

	// check that directory exists, and create it if necessary
	struct stat	sb;
	if (stat(_path.c_str(), &sb) < 0) {
		if (mkdir(_path.c_str(), 0777) < 0) {
			std::string	msg = stringprintf("cannot create "
				"%s: %s", _path.c_str(), strerror(errno));
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
	} else {
		if (!S_ISDIR(sb.st_mode)) {
			std::string	msg = stringprintf("%s exists but is "
				"not a directory", _path.c_str());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
	}

	// set default timestamp format
	if (_format == TIMESTAMP) {
		_timestampformat = "%Y%m%d-%H%M%S";
	}

	// ensure the index file exists
	indexfile = stringprintf("%s/index", _path.c_str());
	if (stat(indexfile.c_str(), &sb) < 0) {
		std::ofstream	out(indexfile.c_str());
		out << 0 << std::endl;;
		out.close();
	}
}

/**
 * \brief Construct a new FITSdirectory in the current working directory
 */
FITSdirectory::FITSdirectory(filenameformat format)
	: _path("."), _format(format) {
	setup();
}

/**
 * \brief Construct a new FITSdirectory in a given path
 */
FITSdirectory::FITSdirectory(const std::string& path, filenameformat format)
	: _path(path), _format(format) {
	setup();
}

/**
 * \brief Construct a new FITSdirectory based on a date
 *
 * This method creates a directory from the time using the prefix/YYYY/mm/dd
 * format.
 */
FITSdirectory::FITSdirectory(const std::string& prefix, const time_t when,
	filenameformat format) : _format(format) {
	// some auxiliary variables we need for initialization
	struct stat	sb;
	char	p[1024];

	// get local time of timestamp
	struct tm	*lt = localtime(&when);

	// year directory
	snprintf(p, sizeof(p), "%s/%04d", prefix.c_str(), lt->tm_year + 1900);
	if ((stat(p, &sb) < 0) && (errno == ENOENT)) {
		if (mkdir(p, 0777) < 0) {
			std::string	msg = stringprintf(
				"cannot create year directory %s: %s",
				p, strerror(errno));
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
	}

	// month directory
	snprintf(p, sizeof(p), "%s/%04d/%02d", prefix.c_str(),
		lt->tm_year + 1900, lt->tm_mon + 1);
	if ((stat(p, &sb) < 0) && (errno == ENOENT)) {
		if (mkdir(p, 0777) < 0) {
			std::string	msg = stringprintf(
				"cannot create month directory %s: %s",
				p, strerror(errno));
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
	}

	// day directory
	snprintf(p, sizeof(p), "%s/%04d/%02d/%02d", prefix.c_str(),
		lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday);
	if ((stat(p, &sb) < 0) && (errno == ENOENT)) {
		if (mkdir(p, 0777) < 0) {
			std::string	msg = stringprintf(
				"cannot create day directory %s: %s",
				p, strerror(errno));
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
	}
	_path = std::string(p);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "path set to: %s", _path.c_str());

	// now do the normal setup
	setup();
}

/**
 * \brief Add an image file to the directory
 *
 * This method locks the index file, reads the contents from it, creates 
 * a suiteable file name, writes the image to the new file name, and 
 * unlocks the index file. This ensures that even concurrently accessing
 * writers will get different file names.
 */
std::string	FITSdirectory::add(const ImagePtr image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding image");
	// lock the index file
	int	fd = open(indexfile.c_str(), O_RDONLY);
	if (flock(fd, LOCK_EX) < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot lock index file, proceed at your own peril");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "lock acquired");

	// read the number from the index file
	std::ifstream	in(indexfile.c_str());
	unsigned int	index;
	in >> index;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "index = %d", index);
	in.close();

	// increment the index and write it to the index file
	index++;
	std::ofstream	out(indexfile.c_str());
	out << index << std::endl;
	out.close();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new value written to index file");

	// construct the filename
	std::string	filename;
	if (_format == COUNTER) {
		filename = stringprintf("%s/%05d.fits", _path.c_str(), index);
	} else {
		// build a timestamp as the filename, without the extension
		char	buffer[1024];
		time_t	now = time(NULL);
		struct tm	*lt = localtime(&now);
		strftime(buffer, sizeof(buffer), _timestampformat.c_str(), lt);
		if (_format == BOTH) {
			filename = stringprintf("%s/%05d-%s", _path.c_str(),
				index, buffer);
		} else {
			filename = stringprintf("%s/%s", _path.c_str(), buffer);
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "file base: %s",
			filename.c_str());

		// it could happen that the file name already exists, so
		// we add counter suffixes until we find a name that does not
		// exist yet
		struct stat	sb;
		int	suffix = 0;
		std::string	suffixedfilename = filename + ".fits";
		while (0 == stat(suffixedfilename.c_str(), &sb)
			|| (errno != ENOENT)) {
			suffix++;
			suffixedfilename = stringprintf("%s-%d.fits",
				filename.c_str(), suffix);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "trying %s",
				suffixedfilename.c_str());
		}

		// the last suffixed filename is ok for writing
		filename = suffixedfilename;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filename: %s", filename.c_str());

	// create a new file
	unlink(filename.c_str());
	FITSout	fitsout(filename);
	fitsout.write(image);

	// unlock the index file
	flock(fd, LOCK_UN);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "lock released");
	close(fd);

	// return the file name
	return filename;
}


} // namespace io
} // namespace astro 
