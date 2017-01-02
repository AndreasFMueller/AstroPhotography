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
 * \brief Set the base directory of the image directory
 */
void	ImageDirectory::basedir(const std::string& b) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting base directory to %s",
		b.c_str());
	_basedir = b;
}

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

int	ImageDirectory::bytesPerPlane(const std::string& filename) const {
	std::string	f = fullname(filename);
	io::FITSinfileBase	infile(f);

	switch (infile.getImgtype()) {
	case BYTE_IMG:
	case SBYTE_IMG:
		return sizeof(unsigned char);
	case USHORT_IMG:
	case SHORT_IMG:
		return sizeof(unsigned short);
	case ULONG_IMG:
	case LONG_IMG:
		return sizeof(unsigned long);
	case FLOAT_IMG:
		return sizeof(float);
	case DOUBLE_IMG:
		return sizeof(double);
	}
	return 1;
}

/**
 * \brief Get a list of file names
 */
std::list<std::string>	ImageDirectory::fileList() {
	std::list<std::string>	names;
	DIR	*dir = opendir(_basedir.c_str());
	if (NULL == dir) {
		throw std::runtime_error("cannot open directory");
	}
	struct dirent	*d;
	while (NULL != (d = readdir(dir))) {
		std::string	filename(d->d_name);
		if (filename[0] == '.') {
			// skip files that start with a .
			continue;
		}
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
	int	fd = mkstemps(buffer, 5);
	if (fd < 0) {
		std::string	cause = stringprintf("cannot create a tmp "
			"image file: %s", strerror(errno));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw std::runtime_error(cause);
	}
	unlink(buffer);
	close(fd);
	std::string	fullname(buffer);

	// construct the filename
	std::string	filename = basename(fullname);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image full name: %s, filename: %s",
		fullname.c_str(), filename.c_str());

	// write the file
	ImageDirectory::write(image, filename);

	// return the filename
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image short name: %s",
		filename.c_str());
	return filename;
}

/**
 * \brief Overwrite an existing file
 *
 * This method is protected because we don't want other parts of the
 * system to randomly overwrite files
 */
void	ImageDirectory::write(astro::image::ImagePtr image,
		const std::string& filename) {
	std::string	f = fullname(filename);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "write image to file %s, fullname = %s",
		filename.c_str(), f.c_str());

	// actually write the file
	try {
		astro::io::FITSout	outfile(f);
		outfile.setPrecious(false);
		if (outfile.exists()) {
			outfile.unlink();
		}
		outfile.write(image);
	} catch (std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot write file '%s': %s",
			f.c_str(), x.what());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image file %s written", f.c_str());
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "file %s removed (unlink)",
		filename.c_str());
}

/**
 * \brief retrieve an image from the image directory
 */
ImagePtr	ImageDirectory::getImagePtr(const std::string& filename) {
	astro::io::FITSin	in(fullname(filename));
	return in.read();
}

/**
 * \brief Get a meta value from an image
 */
Metavalue	ImageDirectory::getMetadata(const std::string& filename,
			const std::string& keyword) {
	return getImagePtr(filename)->getMetadata(keyword);
}

/**
 * \brief Set the meta data in an image
 */
void	ImageDirectory::setMetadata(const std::string& filename,
		const ImageMetadata& metadata) {
	ImagePtr	image = getImagePtr(filename);
	for (auto ptr = metadata.begin(); ptr != metadata.end(); ptr++) {
		image->setMetadata(ptr->second);
	}
	write(image, filename);
}

} // namespace image
} // namespace Astro
