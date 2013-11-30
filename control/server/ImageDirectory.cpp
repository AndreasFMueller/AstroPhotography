/*
 * ImageDirectory.cpp -- directory containing images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ImageDirectory.h>
#include <sys/stat.h>
#include <time.h>
#include <AstroDebug.h>
#include <dirent.h>
#include <OrbSingleton.h>
#include <unistd.h>
#include <AstroIO.h>
#include <string.h>
#include <errno.h>

namespace Astro {

/**
 * \brief static constant base directory name
 */
std::string	ImageDirectory::_basedir("/tmp");

/**
 * \brief Build the full name from a 
 */
std::string	ImageDirectory::fullname(const std::string& filename) const {
	return basedir() + "/" + filename;
}

/**
 * \brief Test whether a file exists
 */
bool	ImageDirectory::isFile(const std::string& filename) const {
	std::string	fn = fullname(filename);
	struct stat	sb;
	if (stat(fn.c_str(), &sb) < 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot stat file %s: %s",
			fn.c_str(), strerror(errno));
		return false;
	}
	return S_ISREG(sb.st_mode) ? true : false;
}

/**
 * \brief Get the size of the file
 */
long	ImageDirectory::fileSize(const std::string& name) const {
	std::string	fn = fullname(name);
	struct stat	sb;
	if (stat(fn.c_str(), &sb) < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "file %s does not exist: %s",
			fn.c_str(), strerror(errno));
		return -1;
	}
	return sb.st_size;
}

/**
 * \brief Get the age of the file
 */
long	ImageDirectory::fileAge(const std::string& name) const {
	std::string	fn = fullname(name);
	struct stat	sb;
	if (stat(fn.c_str(), &sb) < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "file %s does not exist: %s",
			fn.c_str(), strerror(errno));
		return -1;
	}
	long	age = time(NULL) - sb.st_ctime;
	return age;
}

/**
 * \brief Get a list of file names
 */
std::list<std::string>	ImageDirectory::fileList() const {
	std::list<std::string>	names;
	DIR	*dir = opendir(_basedir.c_str());
	if (NULL == dir) {
		throw std::runtime_error("cannot open directory");
	}
	struct dirent	*d;
	while (NULL != (d = readdir(dir))) {
		std::string	filename(d->d_name, d->d_reclen);
		if (isFile(filename)) {
			names.push_back(filename);
		}
	}
	closedir(dir);

	// now convert this into a corba list
	return names;
}

/**
 * \brief Get an image object reference
 */
Image_ptr	ImageDirectory::getImage(const std::string& filename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "registering object id for %s",
		filename.c_str());
	// check whether the file exists
	if (!isFile(filename)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image %s does not exist",
			filename.c_str());
		NotFound	notfound;
		notfound.cause = CORBA::string_dup("file does not exist");
		throw notfound;
	}

	// create an object id associated with the file name
	PortableServer::ObjectId_var	oid
		= PortableServer::string_to_ObjectId(filename.c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "oid created");
	
	// now create an object reference in the POA for images
	OrbSingleton	orb;
	PoaName	poapath("Images");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getting POA for Images");
	PortableServer::POA_var	images_poa = orb.findPOA(poapath);
	CORBA::Object_var	obj
		= images_poa->create_reference_with_id(oid,
			"IDL:/Astro/Image");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reference for image created");
	return Image::_narrow(obj);
}

/**
 * \brief Get the base filename from a path
 */
static std::string	basename(const std::string& fullname) {
	size_t	offset = fullname.rfind('/');
	return fullname.substr(offset + 1);
}

/**
 * \brief Save an image in the directory, return the short name
 */
std::string	ImageDirectory::save(astro::image::ImagePtr image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "saving an image");
	// create a temporary file name in the base directory
	char	buffer[1024];
	snprintf(buffer, sizeof(buffer), "%s/XXXXXXXX.fits", basedir().c_str());
	mkstemps(buffer, 5);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image file name: %s", buffer);

	// write the file
	std::string	fullname(buffer);
	astro::io::FITSout	outfile(fullname);
	outfile.setPrecious(false);
	outfile.write(image);

	// construct the filename
	std::string	filename = basename(fullname);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image short name: %s",
		filename.c_str());
	return filename;
}

} // namespace Astro
