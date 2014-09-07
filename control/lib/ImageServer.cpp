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
ImageServer::ImageServer(Database database, const std::string& directory,
	bool scan)
	: _database(database), _directory(directory) {
	// scan the directory for 
	if (scan) {
		scan_directory();
	}
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d metadata records added", seqno);
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
	return ImageServerTable(_database).byid(id).filename;
}

std::string	ImageServer::pathname(long id) {
	return _directory + "/" + filename(id);
}

/**
 * \brief Get an image
 */
ImagePtr	ImageServer::getImage(long id) {
	std::string	f = pathname(id);
	FITSin	in(f);
	return in.read();
}

/**
 * \brief Retrieve the metadata for an image
 */
ImageEnvelope	ImageServer::getEnvelope(long id) {
	// create a result record
	ImageEnvelope	result(id);

	// read the global information from the database
	ImageServerRecord	imageserverinfo
		= ImageServerTable(_database).byid(id);
	result._size = ImageSize(imageserverinfo.width, imageserverinfo.width);

	// retrieve all the metadata available
	std::string	condition = stringprintf("imageid = %ld", id);
	std::list<MetadataRecord>	mdrecords
		= MetadataTable(_database).select(condition);

	// convert the MetadataRecords into actual metadata
	std::list<MetadataRecord>::const_iterator	mi;
	for (mi = mdrecords.begin(); mi != mdrecords.end(); mi++) {
		Metavalue	m = FITSKeywords::meta(mi->key, mi->value,
					mi->comment);
		result.metadata.setMetadata(m);
	}

	// we are done, return the envelope
	return result;
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
long	ImageServer::save(ImagePtr image) {
	// first we have to create a file name for the image
	char	buffer[MAXPATHLEN];
	snprintf(buffer, sizeof(buffer), "%s/image-XXXXX.fits",
		_directory.c_str());
	if (mkstemps(buffer, 5) < 0) {
		std::string	msg
			= stringprintf("cannot create a filename: %s",
				strerror(errno));;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	std::string	fullname = std::string(buffer);
	std::string	filename = fullname.substr(_directory.size() + 1);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "full name: %s", fullname.c_str());

	// write the image
	unlink(fullname.c_str());
	FITSout	out(fullname);
	out.write(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image written to %s", fullname.c_str());

	// build imageserverinfo reocord
	ImageServerRecord	imageserverinfo(-1);
	imageserverinfo.filename = filename;
	try {
		imageserverinfo.project
			= (std::string)image->getMetadata("PROJECT");
	} catch (...) { }
	imageserverinfo.width = image->size().width();
	imageserverinfo.height = image->size().height();
	imageserverinfo.depth = image->planes();
	imageserverinfo.pixeltype = image->bitsPerPlane();
	try {
		imageserverinfo.exposuretime
			= (double)image->getMetadata("EXPTIME");
	} catch (...) { }
	try {
		imageserverinfo.temperature
			= (double)image->getMetadata("CCD-TEMP");
	} catch (...) { }
	try {
		imageserverinfo.category
			= (std::string)image->getMetadata("PURPOSE");
	} catch (...) { }
	try {
		imageserverinfo.bayer
			= (std::string)image->getMetadata("BAYER");
	} catch (...) { }
	try {
		imageserverinfo.observation
			= (std::string)image->getMetadata("DATE-OBS");
	} catch (...) { }

	// save the imageserver info
	ImageServerTable	images(_database);
	long	imageid = images.add(imageserverinfo);

	// write the metadata to the metadata tabe
	MetadataTable	metadata(_database);
	
	// now add an entry for each meta data record
	ImageMetadata::const_iterator	mi;
	long	seqno = 0;
	for (mi = image->begin(); mi != image->end(); mi++) {
		MetadataRecord	m(-1, imageid);
		m.seqno = seqno;
		m.key = mi->first;
		m.value = mi->second.getValue();
		m.comment = mi->second.getComment();
		metadata.add(m);
		seqno++;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d metadata records added", seqno);

	// done
	return imageid;
}

/**
 * \brief Remove the image and the metadata from the database
 */
void	ImageServer::remove(long id) {
	ImageServerTable(_database).remove(id);
}

} // namespace project
} // namespace astro
