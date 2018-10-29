/*
 * FITS.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroImage.h>
#include <AstroIO.h>
#include <includes.h>
#include <unistd.h>

namespace astro {
namespace image {

/*
 * \brief FITS constructor
 */
FITS::FITS() { }

/**
 * \brief Find out whether this is a FITS file
 *
 * \param filename	name of the file
 */
bool	FITS::isfitsfilename(const std::string& filename) {
	if (filename.size() > 5) {
		if (filename.substr(filename.size() - 5)
			== std::string(".fits")) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "filename %s is FITS",
				filename.c_str());
			return true;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s is not FITS filename",
		filename.c_str());
	return false;
}

/**
 * \brief Write an image to a file
 *
 * \param image		The image that should be written
 * \param filename	The filename of the file to be written
 */
size_t	FITS::write(ImagePtr image, const std::string& filename) {
	io::FITSout	out(filename);
	out.setPrecious(false);
	out.write(image);
	struct stat	sb;
	if (stat(filename.c_str(), &sb) < 0) {
		return 0;
	}
	return sb.st_size;
}

/**
 * \brief Write a FITS image
 *
 * \param image		Image to write
 * \param filename	Name of the file
 */
size_t	FITS::writeFITS(ImagePtr image, const std::string& filename) {
	return write(image, filename);
}

/**
 * \brief Write an image to a buffer
 *
 * \param image		the image to write
 * \param buffer	the pointer to the buffer
 * \param buffersize	the size of the buffer
 */
size_t	FITS::writeFITS(ImagePtr image, void **buffer, size_t *buffersize) {
	char	filename[1024];
	tmpnam(filename);

	// write the image to the temporary file
	ssize_t	s = write(image, std::string(filename));

	// prepare a buffer
	unsigned char	*b = (unsigned char *)malloc(s);
	if (NULL == b) {
		std::string	msg = stringprintf("cannot allocate: %s",
			strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		unlink(filename);
		throw std::runtime_error(msg);
	}
	int	fd = open(filename, O_RDONLY);
	if (s != ::read(fd, b, s)) {
		
	}
	close(fd);
	unlink(filename);

	// copy the data
	*buffer = b;
	*buffersize = s;
	return s;
}

/**
 * \brief Read an Image from a file
 *
 * This is an operation that we have always been able to perform,
 * just with a slightly different API
 *
 * \param filename	name of the file
 */
ImagePtr	FITS::readFITS(const std::string& filename) {
	io::FITSin	in(filename);
	ImagePtr	image = in.read();
	return image;
}

/**
 * \brief Read a FITS file from a buffer
 *
 * \param buffer	buffer containing the image data in FITS format
 * \param buffersize	size of the buffer to read
 */
ImagePtr	FITS::readFITS(void *buffer, size_t buffersize) {
	char	filename[1024];
	if (getenv("TMPDIR")) {
		snprintf(filename, sizeof(filename), "%s/tempXXXXXX.fits",
			getenv("TMPDIR"));
	} else {
		strcpy(filename, "/tmp/tempXXXXXX.fits");
	}

	// open the temporary file
	int	fd = mkstemps(filename, 5);
	if (fd < 0) {
		std::string	msg = stringprintf("cannot create %s: %s",
			filename, strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "using temp file %s", filename);

	// write the data to the file
	ssize_t	rc = ::write(fd, buffer, buffersize);
	if ((rc < 0) || (buffersize != (size_t)rc)) {
		std::string	msg = stringprintf("cannot write %s: %s",
			filename, strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		close(fd);
		throw std::runtime_error(msg);
	}
	close(fd);

	// read the image back from the file
	io::FITSin	in(filename);
	ImagePtr	image = in.read();

	// unlink the file
	unlink(filename);

	// return the image
	return image;
}

} // namespace image
} // namespace astro
