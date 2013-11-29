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

namespace Astro {

std::string	ImageDirectory::_basedir("/tmp");

std::string	ImageDirectory::fullname(const std::string& filename) const {
	return _basedir + "/" + filename;
}

bool	ImageDirectory::isFile(const std::string& filename) {
	std::string	fn = fullname(filename);
	struct stat	sb;
	if (stat(fn.c_str(), &sb) < 0) {
		return false;
	}
	return S_ISREG(sb.st_mode) ? true : false;
}

long	ImageDirectory::fileSize(const std::string& name) {
	std::string	fn = fullname(name);
	struct stat	sb;
	if (stat(fn.c_str(), &sb) < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "file %s does not exist",
			fn.c_str());
		return -1;
	}
	return sb.st_size;
}

long	ImageDirectory::fileAge(const std::string& name) {
	std::string	fn = fullname(name);
	struct stat	sb;
	if (stat(fn.c_str(), &sb) < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "file %s does not exist",
			fn.c_str());
		return -1;
	}
	long	age = time(NULL) - sb.st_ctime;
	return age;
}

std::list<std::string>	ImageDirectory::fileList() {
	std::list<std::string>	names;
	DIR	*dir = opendir(_basedir.c_str());
	if (NULL == dir) {
		throw std::runtime_error("cannot open directory");
	}
	struct dirent	*d;
	while (NULL != (d = readdir(dir))) {
		std::string	filename(d->d_name, d->d_namlen);
		if (isFile(filename)) {
			names.push_back(filename);
		}
	}
	closedir(dir);

	// now convert this into a corba list
	return names;
}

Image_ptr	ImageDirectory::getImage(const std::string& filename) {
	// check whether the file exists
	if (!isFile(filename)) {
		NotFound	notfound;
		notfound.cause = CORBA::string_dup("file does not exist");
		throw notfound;
	}

	// create an object id associated with the file name
	std::string	oidstr = filename;
	PortableServer::ObjectId_var	oid
		= PortableServer::string_to_ObjectId(oidstr.c_str());
	
	// now create an object reference in the POA for images
	OrbSingleton	orb;
	PoaName	poapath("Images");
	PortableServer::POA_var	images_poa = orb.findPOA(poapath);
	CORBA::Object_var	obj
		= images_poa->create_reference_with_id(oid,
			"IDL:/Astro/Image");
	return Image::_narrow(obj);
}

std::string	ImageDirectory::save(astro::image::ImagePtr image) {
	// create a temporary file name in the base directory
	char	buffer[1024];
	snprintf(buffer, sizeof(buffer), "%s/XXXXXXXX.fits",
		basedir().c_str());
	mkstemps(buffer, 5);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image file name: %s", buffer);

	// write the file
	std::string	fullname(buffer);
	astro::io::FITSout	outfile(fullname);
	outfile.write(image);

	// construct the filename
	std::string	filename = fullname.substr(fullname.size() - 12);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image short name: %s",
		filename.c_str());
	return filename;
}

} // namespace Astro
