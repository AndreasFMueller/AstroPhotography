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
		(_format == TIMESTAMP) ? "timestamp" : "counter", path.c_str());

	// check that directory exists, and create it if necessary
	struct stat	sb;
	if (stat(path.c_str(), &sb) < 0) {
		if (mkdir(path.c_str(), 0777) < 0) {
			std::string	msg = stringprintf("cannot create "
				"%s: %s", path.c_str(), strerror(errno));
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
	} else {
		if (!S_ISDIR(sb.st_mode)) {
			std::string	msg = stringprintf("%s exists but is "
				"not a directory", path.c_str());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
	}

	// set default timestamp format
	if (_format == TIMESTAMP) {
		_timestampformat = "%Y%m%d-%H%M%S";
	}

	// ensure the index file exists
	indexfile = stringprintf("%s/index", path.c_str());
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
	: path("."), _format(format) {
	setup();
}

/**
 * \brief Construct a new FITSdirectory in a given path
 */
FITSdirectory::FITSdirectory(const std::string& _path, filenameformat format)
	: path(_path), _format(format) {
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
void	FITSdirectory::add(const ImagePtr& image) {
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
		filename = stringprintf("%s/%05d.fits", path.c_str(), index);
	} else {
		// build a timestamp as the filename, without the extension
		char	buffer[1024];
		time_t	now = time(NULL);
		struct tm	*lt = localtime(&now);
		strftime(buffer, sizeof(buffer), _timestampformat.c_str(), lt);
		filename = stringprintf("%s/%s", path.c_str(), buffer);
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
}


} // namespace io
} // namespace astro 
