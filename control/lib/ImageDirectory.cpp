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
#include <unistd.h>
#include <AstroIO.h>
#include <string.h>
#include <errno.h>

namespace astro {
namespace image {

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
 * \brief Get the pixel size
 * 
 * For this we read the headers of the FITS file, and derive the size from
 * the header information.
 */
int	ImageDirectory::bytesPerPixel(const std::string& filename) const {
	std::string	f = fullname(filename);
	io::FITSinfileBase	infile(f);

	switch (infile.getImgtype()) {
	case BYTE_IMG:
	case SBYTE_IMG:
		return sizeof(unsigned char) * infile.getPlanes();
	case USHORT_IMG:
	case SHORT_IMG:
		return sizeof(unsigned short) * infile.getPlanes();
	case ULONG_IMG:
	case LONG_IMG:
		return sizeof(unsigned long) * infile.getPlanes();
	case FLOAT_IMG:
		return sizeof(float) * infile.getPlanes();
	case DOUBLE_IMG:
		return sizeof(double) * infile.getPlanes();
	}
	return 2;
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
	unlink(buffer);

	// write the file
	std::string	fullname(buffer);
	try {
		astro::io::FITSout	outfile(fullname);
		outfile.setPrecious(false);
		outfile.write(image);
	} catch (std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot write file '%s': %s",
			fullname.c_str(), x.what());
	}

	// construct the filename
	std::string	filename = basename(fullname);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image short name: %s",
		filename.c_str());
	return filename;
}

/**
 * \brief Remove an image from the directory
 */
void	ImageDirectory::remove(const std::string& filename) {
	if (!isFile(filename)) {
		throw std::runtime_error("file not found");
	}
	if (unlink(fullname(filename).c_str()) < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot remove %s: %s",
			filename.c_str(), strerror(errno));
		throw std::runtime_error("cannot remove file");
	}
}

/**
 * \brief retrieve an image from the image directory
 */
ImagePtr	ImageDirectory::getImagePtr(const std::string& filename) {
	astro::io::FITSin	in(fullname(filename));
	return in.read();
}

} // namespace image
} // namespace Astro
