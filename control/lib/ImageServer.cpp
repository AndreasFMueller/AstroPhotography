/*
 * ImageServer.cpp -- implementation of the image server
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProject.h>
#include <AstroDebug.h>
#include <AstroIO.h>
#include <includes.h>
#include <ImageServerTables.h>

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
	// scan the directory for 
	scan_directory();
}

/**
 * \brief get the id of an image identified by its filename
 */
long	ImageServer::id(const std::string& filename) {
	ImageServerTable	images(_database);
	return images.id(filename);
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
	ImageServerTable	images(_database);
	try {
		long	id = images.id(filename);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s exists with id %ld", 
			filename.c_str(), id);
		// if we get to this point, the file already exists, so we
		// skip it
		return;
	} catch (...) {
		// file does not exist
	}

	// read the metadata
        io::FITSinfileBase      infile(fullname);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "size: %s",
		infile.getSize().toString().c_str());

	// create an information record
	ImageServerRecord	imageserverinfo;
	imageserverinfo.filename = filename;
	imageserverinfo.project = "unknown";
	imageserverinfo.created = sb.st_ctime;
	imageserverinfo.width = infile.getSize().width();
	imageserverinfo.height = infile.getSize().height();
	imageserverinfo.depth = infile.getPlanes();
	imageserverinfo.pixeltype = infile.getPixeltype();
	imageserverinfo.exposuretime = 0;
	try {
		imageserverinfo.exposuretime
			= (double)infile.getMetadata("EXPTIME");
	} catch(...) { }
	imageserverinfo.temperature = 0;
	try {
		imageserverinfo.temperature
			= (double)infile.getMetadata("CCD-TEMP");
	} catch(...) { }
	imageserverinfo.category = "light";
	imageserverinfo.bayer = "    ";
	imageserverinfo.observation = "1970-01-01T00:00:00.000";

	// add the entry to the table
	long	imageid = images.add(imageserverinfo);

	// in the part below we need the metatdata table
	MetadataTable	metadatatable(_database);

	// get all the metadata from the infile
	int	seqno = 0;
	ImageMetadata	md = infile.getAllMetadata();

	// now add an entry for each meta data record
	ImageMetadata::const_iterator	mi;
	for (mi = md.begin(); mi != md.end(); mi++) {
		MetadataRecord	m(-1, imageid);
		m.seqno = seqno;
		m.key = mi->first;
		m.value = mi->second.getValue();
		m.comment = mi->second.getComment();
		metadatatable.add(m);
		seqno++;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d metadata records added");

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

	// set up a counter so we can count the number of files we find
	int	counter = 0;

	// open the directory
	DIR     *dir = opendir(_directory.c_str());
	if (NULL == dir) {
		throw std::runtime_error("cannot open directory");
	}
	struct dirent	*d;
	while (NULL != (d = readdir(dir))) {
		std::string     filename(d->d_name, d->d_namlen);
		scan_file(filename);
		counter++;
	}

	// close the directory
	closedir(dir);

	// report the number of files scanned
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d files scanned", counter);
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
