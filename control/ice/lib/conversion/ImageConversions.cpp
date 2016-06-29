/*
 * ImageConversions.cpp
 *
 * (c) 2014 Prof Dr Andreas Mueller, 
 */
#include <IceConversions.h>
#include <type_traits>
#include <limits>
#include <includes.h>
#include <AstroIO.h>
#include <AstroFormat.h>
#include <typeinfo>

namespace snowstar {

/**
 * \brief Convert an Imge Proxy into an image
 */
astro::image::ImagePtr	convert(ImagePrx image) {
	// get the image data from the server
	ImageFile	file = image->file();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got image of size %d", file.size());

	// construct a temporary file name
	char	buffer[1024];
	if (getenv("TMPDIR")) {
		snprintf(buffer, sizeof(buffer), "%s/convert-XXXXXX.fits",
			getenv("TMPDIR"));
	} else {
		strcpy(buffer, "/tmp/convert-XXXXXX.fits");
	}
	if (mkstemps(buffer, 5) < 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot create temp file: %s",
			strerror(errno));
		throw std::runtime_error("cannot create tmp file name");
	}
	std::string	filename(buffer);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "temporary image file: %s", buffer);

	// write image data to a temporary file
	int	out = open(buffer, O_CREAT | O_WRONLY, 0666);
	if (out < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot create temporary file %s",
			buffer);
		throw std::runtime_error("cannot create temp file");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "out fd = %d", out);
	int	s = file.size();
	if (s != write(out, file.data(), s)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "writing temp file failed: %s",
			strerror(errno));
	}
	close(out);

	// use FITS classes to read the temporary file
	astro::io::FITSin	in(filename);
	astro::image::ImagePtr	result = in.read();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got an %s image with pixel type %s",
		result->size().toString().c_str(),
		astro::demangle(result->pixel_type().name()).c_str());

	// unlink the temporary file
	unlink(filename.c_str());

	// return the image we just read
	return result;
}


/**
 * \brief Convert a simple image to a astro::image::Image
 */
astro::image::ImagePtr	convertsimple(SimpleImage image) {
	astro::image::ImageSize	size(image.size.width, image.size.height);
	astro::image::Image<unsigned short>	*imageptr
		= new astro::image::Image<unsigned short>(size);
	long	length = image.size.width * image.size.height;
	for (int offset = 0; offset < length; offset++) {
		(*imageptr)[offset] = image.imagedata[offset];
	}
	return astro::image::ImagePtr(imageptr);
}

/**
 * \brief An adapter template class to convert to unsigned short pixels
 */
template<typename Pixel>
class UnsignedShortAdapter
	: public astro::image::ConstImageAdapter<unsigned short> {
	const astro::image::ConstImageAdapter<Pixel>&	image;
public:
	UnsignedShortAdapter(const astro::image::ConstImageAdapter<Pixel>&
		_image);
	virtual unsigned short	pixel(int x, int y) const;
};

/**
 * \brief Constructor for the Unsigned short adapter
 */
template<typename Pixel>
UnsignedShortAdapter<Pixel>::UnsignedShortAdapter(
			const astro::image::ConstImageAdapter<Pixel>& _image)
	: astro::image::ConstImageAdapter<unsigned short>(_image.getSize()),
	  image(_image) {
}

/**
 * \brief Pixel accessor for the unsigned short adapter
 *
 */
template<typename Pixel>
unsigned short	UnsignedShortAdapter<Pixel>::pixel(int x, int y) const {
	unsigned short	value;
	// code to be compiled for floating point pixel types
	if (std::is_floating_point<Pixel>()) {
		value = std::numeric_limits<unsigned short>::max()
			* image.pixel(x, y);
	}
	// code to be compiled for integral pixel types
	if (std::is_integral<Pixel>()) {
		// if the pixel has more pixels that unsigned short...
		if (std::numeric_limits<Pixel>::digits > 16) {
			// ... then we shift the value to the right and ..
			value = image.pixel(x, y) >>
				(std::numeric_limits<Pixel>::digits - 16);
		} else {
			// ... otherwise we shift to the left
			value = image.pixel(x, y) << 
				(16 - std::numeric_limits<Pixel>::digits);
		}
	}
	return value;
}

#define	get_reduction(Pixel)						\
{									\
	astro::image::Image<Pixel >	*dimage				\
		= dynamic_cast<astro::image::Image<Pixel > *>(&*image);	\
	if (NULL != dimage) {						\
		reduction = new UnsignedShortAdapter<Pixel >(*dimage);	\
	}								\
}


/**
 * \brief Convert an astro::image::Image to a SimpleImage
 */
SimpleImage	convertsimple(astro::image::ImagePtr image) {
	SimpleImage	result;

	// convert the size
	result.size = convert(image->size());

	// try to convert directly
	astro::image::Image<unsigned short>	*im
		= dynamic_cast<astro::image::Image<unsigned short> *>(&*image);
	if (NULL != im) {
		for (int x = 0; x < result.size.width; x++) {
			for (int y = 0; y < result.size.width; y++) {
				unsigned short	value = im->pixel(x, y);
				result.imagedata.push_back(value);
			}
		}
		return result;
	}

	// create an image adapter that allows us to read unsigned short
	// values from the image
	astro::image::ConstImageAdapter<unsigned short>	*reduction = NULL;
	get_reduction(unsigned char)
	get_reduction(unsigned long)

	// convert the image data
	if (NULL == reduction) {
		throw std::runtime_error("no reduction found");
	}

	// if we get to this point, then a suitable reduction exists and
	// we can use it to convert pixel values to unsigned short
	for (int x = 0; x < result.size.width; x++) {
		for (int y = 0; y < result.size.width; y++) {
			unsigned short	value = reduction->pixel(x, y);
			result.imagedata.push_back(value);
		}
	}

	return result;
}

/**
 * \brief auxiliary function ot create a temporary file
 */
static std::string	tempfilename() {
	const char	*tmpdir = "/tmp";
	if (NULL != getenv("TMPDIR")) {
		tmpdir = getenv("TMPDIR");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "temp an image");
	char    buffer[1024];
	snprintf(buffer, sizeof(buffer), "%s/XXXXXXXX.fits", tmpdir);
	mkstemps(buffer, 5);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image file name: %s", buffer);
	unlink(buffer);

	return std::string(buffer);
}

/**
 * \brief Convert an ImageFile buffer to an ImagePtr
 */
astro::image::ImagePtr  convertfile(ImageFile imagefile) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "imagefile has size %d",
		imagefile.size());

	// here is the result image we would like to return
	astro::image::ImagePtr	result;

	// generate a temporary file name
	std::string	filename = tempfilename();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get the filename: %s",
		filename.c_str());

	try {
		// open the temporary file
		int	fd = open(filename.c_str(),
				O_CREAT | O_TRUNC | O_WRONLY, 0666);
		if (fd < 0) {
			std::string	msg = astro::stringprintf("cannot "
				"create temporary file: %s", strerror(errno));
			throw std::runtime_error(msg);
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "fd: %d", fd);

		// write the image data to a temporary file
		ssize_t	l = write(fd, imagefile.data(), imagefile.size());
		if (l < 0) {
			std::string	msg = astro::stringprintf("could not "
				"write data: %s", strerror(errno));
			close(fd);
			throw std::runtime_error(msg);
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "bytes written: %d", l);
		if ((ssize_t)imagefile.size() != l) {
			throw std::runtime_error("failed to write image");
		}
		close(fd);

		// read the image
		astro::io::FITSin	in(filename);
		result = in.read();
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "failed to convert: %s (%s)",
			x.what(), typeid(x).name());
	} catch (...) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "exception during conversion");
	}

	// unlink the temporary file
	unlink(filename.c_str());

	// throw an exception if the image is NULL
	if (NULL == result) {
		throw std::runtime_error("cannot convert image");
	}
	return result;
}

/**
 * \brief Convert an ImagePtr into an ImageFile
 */
ImageFile       convertfile(astro::image::ImagePtr imageptr) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "convert image of size %dx%d",
		imageptr->size().width(), imageptr->size().height());
	// generate a temporary file name
	std::string	filename = tempfilename();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tempfile: %s", filename.c_str());

	// write the image to a temporary file
	try {
		astro::io::FITSout	out(filename);
		out.setPrecious(false);
		out.write(imageptr);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot write image: %s",
			x.what());
		throw;
	}

	// find out how large the image is
	struct stat	sb;
	int	rc = stat(filename.c_str(), &sb);
	if (rc < 0) {
		throw NotFound("cannot stat temp image");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image file has size %d", sb.st_size);

	// empty image
	if (0 == sb.st_size) {
		return ImageFile();
	}

	// read the image into a buffer
	int	fd = open(filename.c_str(), O_RDONLY);
	if (fd < 0) {
		throw NotFound("temporary file disappeared");
	}

	unsigned char	*buffer = new unsigned char[sb.st_size];
	if (sb.st_size != read(fd, buffer, sb.st_size)) {
		delete[] buffer;
		close(fd);
		std::string	msg = astro::stringprintf("cannt read file %s in full length %ld", filename.c_str(), sb.st_size);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw BadParameter(msg);
	}
	close(fd);

	// create an image file object
	ImageFile	result(sb.st_size);
	for (int i = 0; i < sb.st_size; i++) {
		result[i] = buffer[i];
	}

	// clean up memory
	delete[] buffer;

	// unlink the image
	unlink(filename.c_str());

	// read the image into a 
	return result;
}

} // namespace snowstar
