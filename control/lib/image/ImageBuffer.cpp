/*
 * ImageBuffer.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroImage.h>
#include <includes.h>

namespace astro {
namespace image {

/**
 * \brief Create an ImageBuffer from a memory buffer
 *
 * Note that this constructor takes ownership of the the buffer
 */
ImageBuffer::ImageBuffer(type_t type, void *buffer, size_t buffersize)
	: Format(type), _buffer(buffer), _buffersize(buffersize) {
}

/**
 * \brief Create an ImageBuffer from an in Memory image
 *
 * \param image		the image that should be written to the buffer
 */
ImageBuffer::ImageBuffer(ImagePtr image) : _buffer(NULL), _buffersize(0) {
	image::FITS	fits;
	fits.writeFITS(image, &_buffer, &_buffersize);
}

/**
 * \brief Create an ImageBuffer from an in memory image with a given type
 *
 * \param image		image to write to memory buffer
 * \param type		type of buffer contents
 */
ImageBuffer::ImageBuffer(ImagePtr image, type_t type)
	: Format(type), _buffer(NULL), _buffersize(0) {
	switch (type) {
	case FITS:
		{
			image::FITS	fits;
			fits.writeFITS(image, &_buffer, &_buffersize);
		}
		break;
	case JPEG:
		{
			image::JPEG	jpeg;
			jpeg.writeJPEG(image, &_buffer, &_buffersize);
		}
		break;
	case PNG:
		{
			image::PNG	png;
			png.writePNG(image, &_buffer, &_buffersize);
		}
		break;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create buffer of size %lu",
		_buffersize);
}

/**
 * \brief Read data from a file
 *
 * \param filename	the name of the file to read
 */
ImageBuffer::ImageBuffer(const std::string& filename) {
	if (FITS::isfitsfilename(filename)) {
		_type = Format::FITS;
	}
	if (PNG::ispngfilename(filename)) {
		_type = Format::PNG;
	}
	if (JPEG::isjpegfilename(filename)) {
		_type = Format::JPEG;
	}
	struct stat	sb;
	if (stat(filename.c_str(), &sb) < 0) {
		std::string	msg = stringprintf("cannot stat %s: %s",
			filename.c_str(), strerror(errno));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	_buffersize = sb.st_size;
	_buffer = (unsigned char *)malloc(sb.st_size);
	if (NULL == _buffer) {
		std::string	msg = stringprintf("cannot allocate: %s",
			strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	int	fd = open(filename.c_str(), O_RDONLY);
	if (fd < 0) {
		free(_buffer);
		std::string	msg = stringprintf("cannot allocate: %s",
			strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	if (_buffersize != ::read(fd, _buffer, _buffersize)) {
		free(_buffer);
		close(fd);
		std::string	msg = stringprintf("cannot read %s: %s",
			filename.c_str(), strerror(errno));
		throw std::runtime_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "read %d bytes, type %d",
		_buffersize, _type);
	close(fd);
}

/**
 * \brief Destroy the buffer
 */
ImageBuffer::~ImageBuffer() {
	if (_buffer) {
		free(_buffer);
	}
	_buffer = NULL;
	_buffersize = 0;
}

/**
 * \brief Retrieve the image from the buffer
 */
ImagePtr	ImageBuffer::image() const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reading image from buffer");
	switch (_type) {
	case Format::FITS:
		{
			image::FITS	fits;
			return fits.readFITS(_buffer, _buffersize);
		}
		break;
	case Format::JPEG:
		{
			image::JPEG	jpeg;
			return jpeg.readJPEG(_buffer, _buffersize);
		}
		break;
	case Format::PNG:
		{
			image::PNG	png;
			return png.readPNG(_buffer, _buffersize);
		}
		break;
	}
}

/**
 * \brief Write data to a file
 *
 * \param filename	name of the file
 */
void	ImageBuffer::write(const std::string& filename) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "writing to file %s, size %d",
		filename.c_str(), _buffersize);
	int	fd = open(filename.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0666);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "fd: %d", fd);
	if (fd < 0) {
		std::string	msg = stringprintf("cannot open file %s: %s",
			filename.c_str(), strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	ssize_t	rc = ::write(fd, _buffer, _buffersize);
	if (rc < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot write %p: %s", _buffer,
			strerror(errno));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d bytes written", rc);
	if (_buffersize != rc) {
		close(fd);
		std::string	msg = stringprintf("cannot write %s: %s",
			filename.c_str(), strerror(errno));
	}
	close(fd);
}

/**
 * \brief Convert the contents of the buffer to a different type
 *
 * \param type		the new type
 */
ImageBuffer	*ImageBuffer::convert(type_t type) const {
	if (type == _type) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no conversion needed");
		void	*b = malloc(_buffersize);
		memcpy(b, _buffer, _buffersize);
		return new ImageBuffer(Format::FITS, b, _buffersize);
	}
	ImagePtr	img = image();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %s", img->info().c_str());
	switch (type) {
	case Format::FITS:
		new ImageBuffer(img);
		break;
	case Format::JPEG:
		{
			debug(LOG_DEBUG, DEBUG_LOG, 0, "converting to JPEG");
			image::JPEG	jpeg;
			void		*b;
			size_t		bs;
			jpeg.writeJPEG(img, &b, &bs);
			return new ImageBuffer(Format::JPEG, b, bs);
		}
		break;
	case Format::PNG:
		{
			debug(LOG_DEBUG, DEBUG_LOG, 0, "converting to PNG");
			image::PNG	png;
			void	 	*b;
			size_t		bs;
			png.writePNG(img, &b, &bs);
			return new ImageBuffer(Format::PNG, b, bs);
		}
		break;
	}
	return NULL;
}

/**
 * \brief Write the buffer contents to a memory buffer
 *
 * \param buffer	the pointer to the newly allocated buffer
 * \param buffersize	size of the buffer
 */
void	ImageBuffer::write(void **buffer, size_t *buffersize) {
	void	*b = malloc(_buffersize);
	memcpy(b, _buffer, _buffersize);
	*buffer = b;
	*buffersize = _buffersize;
}

} // namespace image
} // namespace astro
