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

void	FITSdirectory::setup() {
	struct stat	sb;
	// check that directory exists, and create it if necessary
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

	// ensure the index file exists
	indexfile = stringprintf("%s/index", path.c_str());
	if (stat(indexfile.c_str(), &sb) < 0) {
		std::ofstream	out(indexfile.c_str());
		out << 0 << std::endl;;
		out.close();
	}
}

FITSdirectory::FITSdirectory() : path(".") {
	setup();
}

FITSdirectory::FITSdirectory(const std::string& _path) : path(_path) {
	setup();
}

void	FITSdirectory::add(const ImagePtr& image) {
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

	// create a new file
	std::string	filename
		= stringprintf("%s/%05d.fits", path.c_str(), index);
	unlink(filename.c_str());
	FITSout	fitsout(filename);
	fitsout.write(image);
}


} // namespace io
} // namespace astro 
