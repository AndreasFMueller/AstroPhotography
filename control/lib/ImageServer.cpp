/*
 * ImageServer.cpp -- implementation of the image server
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProject.h>
#include <AstroDebug.h>
#include <AstroIO.h>
#include <includes.h>

using namespace astro::persistence;
using namespace astro::image;
using namespace astro::io;

namespace astro {
namespace project {

/**
 * \brief Create an image server
 */
ImageServer::ImageServer(Database database, const std::string& directory)
	: _database(database), _directory(directory) {
	// ensure that the database has the right tables
	setup_tables();

	// scan the directory for 
	scan_directory();
}

/**
 * \brief process a single file during a scan
 */
void	ImageServer::scan_file(const std::string& filename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "scanning file '%s' (%d)",
		filename.c_str(), filename.length());
	// does the filename end in ".fits"?
	if (filename.size() < 5) {
		return;
	}
	std::string	extension = filename.substr(filename.length() - 5, 5);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "substr: '%s'", extension.c_str());
	if (extension != ".fits") {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s not a FITS file",
			filename.c_str());
		return;
	}

	// check whether this is a file
	std::string	fullname = _directory + "/" + filename;
	struct stat	sb;
	if (stat(fullname.c_str(), &sb) < 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot stat file %s: %s",
			fullname.c_str(), strerror(errno));
		return;
	}

	if (!S_ISREG(sb.st_mode)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s: not a regular file",
			fullname.c_str());
		return;
	}

	// find out whether the database already contains this filename

	// read the metadata
        io::FITSinfileBase      infile(fullname);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "size: %s",
		infile.getSize().toString().c_str());

}

/**
 * \brief Scan a directory for images
 */
void	ImageServer::scan_directory(bool recurse) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "scan directory %s", _directory.c_str());

	if (recurse) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"recursive scan not implemented");
		throw std::runtime_error("not implemented");
		return;
	}

	// open the directory
	DIR     *dir = opendir(_directory.c_str());
	if (NULL == dir) {
		throw std::runtime_error("cannot open directory");
	}
	struct dirent	*d;
	while (NULL != (d = readdir(dir))) {
		std::string     filename(d->d_name, d->d_namlen);
		scan_file(filename);
	}

	// close the directory
	closedir(dir);
	
}

/**
 * \brief Ensure that the 
 */
void	ImageServer::setup_tables() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set up the tables");
}

/**
 * \brief Retrieve an image
 */
std::string	ImageServer::filename(long id) {
	// XXX we should read the name from the database
	return _directory + stringprintf("/%d.fits", id);
}

/**
 * \brief Get an image
 */
ImagePtr	ImageServer::getImage(long id) {
	std::string	f = filename(id);
	FITSin	in(f);
	return in.read();
}

/**
 * \brief Retrieve the metadata for an image
 */
ImageEnvelope	ImageServer::getEnvelope(long id) {
	// XXX this isn't very efficient, we should really read the
	//     metadata from the database
	return ImageEnvelope(getImage(id));
}

/**
 * \brief Get the envelopes that match specification
 */
std::set<ImageEnvelope>	ImageServer::get(const ImageSpec& /* spec */) {
	std::set<ImageEnvelope>	images;
	return images;
}

/**
 * \brief Save an image in the repository
 */
long	ImageServer::save(ImagePtr /* image */) {
	// XXX we should save the image ;-)
	return 0;
}

/**
 * \brief Remove the image and the metadata from the database
 */
void	ImageServer::remove(long /* id */) {
	// XXX remove the image from
}

} // namespace project
} // namespace astro
