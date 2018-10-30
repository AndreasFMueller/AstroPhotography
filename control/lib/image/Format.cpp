/*
 * Format.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroImage.h>

namespace astro {
namespace image {

std::string	Format::typeString() const {
	switch (_type) {
	case Format::FITS:	return std::string("fits");
	case Format::JPEG:	return std::string("jpeg");
	case Format::PNG:	return std::string("png");
	}
}

size_t	Format::write(ImagePtr image, const std::string& filename) {
	if (FITS::isfitsfilename(filename)) {
		image::FITS	fits;
		return fits.writeFITS(image, filename);
	}
	if (JPEG::isjpegfilename(filename)) {
		image::JPEG	jpeg;
		return jpeg.writeJPEG(image, filename);
	}
	if (PNG::ispngfilename(filename)) {
		image::PNG	png;
		return png.writePNG(image, filename);
	}
	std::string	msg = stringprintf("unknown file type '%s'",
				filename.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}


size_t	Format::write(ImagePtr image, Format::type_t type,
		void **buffer, size_t *buffersize) {
	switch (type) {
	case Format::FITS:
		{
			image::FITS	fits;
			fits.writeFITS(image, buffer, buffersize);
		}
		break;
	case Format::JPEG:
		{
			image::JPEG	jpeg;
			jpeg.writeJPEG(image, buffer, buffersize);
		}
		break;
	case Format::PNG:
		{
			image::PNG	png;
			png.writePNG(image, buffer, buffersize);
		}
		break;
	}
	return 0;
}

static int	format_counter = 0;

ImagePtr	Format::read(Format::type_t type,
			void *buffer, size_t buffersize) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d bytes, type %d", buffersize,
		type);
	ImagePtr	result;
	switch (type) {
	case Format::FITS:
		{
			image::FITS	fits;
			result = fits.readFITS(buffer, buffersize);
		}
		break;
	case Format::JPEG:
		{
			image::JPEG	jpeg;
			result = jpeg.readJPEG(buffer, buffersize);
		}
		break;
	case Format::PNG:
		{
			image::PNG	png;
			result = png.readPNG(buffer, buffersize);
		}
		break;
	}
	image::FITS	fits;
	fits.writeFITS(result, stringprintf("test-%d.fits", format_counter++));
	return result;
}

} // namespace image
} // namespace astro
