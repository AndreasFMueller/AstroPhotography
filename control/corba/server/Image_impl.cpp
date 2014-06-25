/*
 * Image_impl.cpp -- CORBA Image wrapper implementation
 * 
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Image_impl.h"

#include <AstroIO.h>
#include <AstroFilterfunc.h>
#include <includes.h>
#include <OrbSingleton.h>

namespace Astro {

/**
 * \brief Construct an Image servant from a file
 */
Image_impl::Image_impl(const std::string& filename) : _filename(filename) {
	_image = getImage();
	// read the image file 
	setup();
}

Image_impl::Image_impl(ImagePtr image) : _image(image) {
	_filename = save(_image);
	setup();
}

Image_impl::~Image_impl() {
	try {
		remove(_filename); // method in ImageDatabaseDirectory
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot remove %s: %s",
			_filename.c_str(), x.what());
	}
}

/**
 * \brief Initialize static fields in the implementation
 */
void	Image_impl::setup() {
	// origin
	_origin.x = _image->origin().x();
	_origin.y = _image->origin().y();
	// size
	_size.width = _image->size().width();
	_size.height = _image->size().height();
	// bytes per pixel
	_bytesperpixel = astro::image::filter::bytesperpixel(_image);
	// bytes per value
	_bytespervalue = astro::image::filter::bytespervalue(_image);
	// planes
	_planes = astro::image::filter::planes(_image);
}

/**
 * \brief Convert image into FITS data
 *
 * Convert the image into a FITS file and then return the contents of the
 * FITS file.
 */
Astro::Image::ImageFile	*Image_impl::file() {
	std::string	fn = fullname(_filename);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "serving file %s", fn.c_str());
	int	fd = open(fn.c_str(), O_RDONLY);
	if (fd < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot open %s: %s",
			fn.c_str(), strerror(errno));
		throw Astro::IOException("cannot image file");
	}
	struct stat	sb;
	int	rc = fstat(fd, &sb);
	if (rc < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot stat %s: %s",
			fn.c_str(), strerror(errno));
		throw Astro::IOException("cannot stat temporary file");
	} 

	// read the data
	CORBA::Octet	*buf = new CORBA::Octet[sb.st_size];
	long	bytes = read(fd, buf, sb.st_size);
	if (bytes != sb.st_size) {
		throw std::runtime_error("incorrect number of bytes read");
	}
	close(fd);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "read %ld bytes", bytes);

	// create an ImageData object
	Astro::Image::ImageFile	*imagefile
		= new Astro::Image::ImageFile(sb.st_size, sb.st_size, buf, 0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ImageData object of length %ld",
		imagefile->length());

	// return the image data
	return imagefile;
}

/**
 * \brief Get the file size
 */
CORBA::Long	Image_impl::filesize() {
	return fileSize(_filename);
}

/**
 * \brief deactivate an object
 *
 * This will also remove the file when the servant is etherialized by
 * the ORB
 */
void	Image_impl::remove() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "removing image %s", _filename.c_str());
	// we need a poa to clean up
	OrbSingleton	orb;
	PortableServer::POA_var	poa = orb.findPOA(PoaName::images());

	// remove the servant 
	PortableServer::ObjectId_var	oid
		= PortableServer::string_to_ObjectId(_filename.c_str());
	poa->deactivate_object(oid);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image %s removed", _filename.c_str());
}

astro::image::ImagePtr	Image_impl::getImage() {
	astro::io::FITSin	infile(fullname(_filename));
	return infile.read();
}

#define	sequence_mono(pixel, size, _image, result)			\
{									\
	astro::image::Image<pixel>	*imagep				\
		= dynamic_cast<astro::image::Image<pixel> *>(&*_image);	\
	if (NULL != imagep) {						\
		for (unsigned int off = 0; off < size; off++) {		\
			(*result)[off] = (*imagep)[off];		\
		}							\
	}								\
}

#define sequence_yuyv(pixel, size, _image, result)			\
{									\
	astro::image::Image<YUYV<pixel> >	*imagep			\
		= dynamic_cast<astro::image::Image<YUYV<pixel> > *>(&*_image);\
	if (NULL != imagep) {						\
		for (unsigned int off = 0; off < size; off++) {		\
			(*result)[2 * off    ] = (*imagep)[off].y;	\
			(*result)[2 * off + 1] = (*imagep)[off].uv;	\
		}							\
	}								\
}

#define sequence_rgb(pixel, size, _image, result)			\
{									\
	astro::image::Image<RGB<pixel> >	*imagep			\
		= dynamic_cast<astro::image::Image<RGB<pixel> > *>(&*_image);\
	if (NULL != imagep) {						\
		for (unsigned int off = 0; off < size; off++) {		\
			(*result)[3 * off    ] = (*imagep)[off].R;	\
			(*result)[3 * off + 1] = (*imagep)[off].G;	\
			(*result)[3 * off + 2] = (*imagep)[off].B;	\
		}							\
	}								\
}

/**
 * \brief Retrieve the raw image data for a byte image
 */
Astro::ByteImage::ByteSequence	*ByteImage_impl::getBytes() {
	Astro::ByteImage::ByteSequence	*result
		= new Astro::ByteImage::ByteSequence();
	astro::image::ImagePtr	_image = getImage();
	unsigned int	size = _image->size().getPixels();
	size_t	bytes = astro::image::filter::planes(_image) * size;
	result->length(bytes);
	sequence_mono(unsigned char, size, _image, result);
	sequence_yuyv(unsigned char, size, _image, result);
	sequence_rgb(unsigned char, size, _image, result);
	return result;
}

ByteImage_impl::~ByteImage_impl() {
}

/**
 * \brief Retrieve the raw image data for a short iamge
 */
Astro::ShortSequence	*ShortImage_impl::getShorts() {
	Astro::ShortSequence	*result = new Astro::ShortSequence();
	astro::image::ImagePtr	_image = getImage();
	unsigned int	size = _image->size().getPixels();
	size_t	shorts = astro::image::filter::planes(_image) * size;
	result->length(shorts);
	sequence_mono(unsigned short, size, _image, result);
	sequence_yuyv(unsigned short, size, _image, result);
	sequence_rgb(unsigned short, size, _image, result);
	return result;
}

ShortImage_impl::~ShortImage_impl() {
}

} // namespace Astro
