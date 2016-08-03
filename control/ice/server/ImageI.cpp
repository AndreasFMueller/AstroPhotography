/*
 * ImageI.cpp -- image servant implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ImageI.h>
#include <AstroFilterfunc.h>
#include <AstroFormat.h>
#include <cerrno>
#include <cstring>
#include <includes.h>
#include <AstroImage.h>
#include <IceConversions.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/Communicator.h>
#include <ProxyCreator.h>
#include <ImagesI.h>

namespace snowstar {

ImageI::ImageI(astro::image::ImageDirectory& imagedirectory,
	astro::image::ImagePtr image, const std::string& filename)
	: _imagedirectory(imagedirectory), _image(image), _filename(filename) {
	// origin
	_origin = convert(_image->origin());
	// size
	_size = convert(_image->size());
	// bytes per pixel
	_bytesperpixel = _image->bytesPerPixel();
	// bytes per value
	_bytespervalue = astro::image::filter::bytespervalue(_image);
	// planes
	_planes = astro::image::filter::planes(_image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image servant created for %s",
		_filename.c_str());
}

ImageI::~ImageI() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy servant for image %s", 
		_filename.c_str());
}

ImageSize       ImageI::size(const Ice::Current& /* current */) {
	return _size;
}

ImagePoint      ImageI::origin(const Ice::Current& /* current */) {
	return _origin;
}

int     ImageI::bytesPerPixel(const Ice::Current& /* current */) {
	return _bytesperpixel;
}

int     ImageI::planes(const Ice::Current& /* current */) {
	return _planes;
}

int     ImageI::bytesPerValue(const Ice::Current& /* current */) {
	return _bytespervalue;
}

ImageFile       ImageI::file(const Ice::Current& /* current */) {
	std::vector<Ice::Byte>	result;
	std::string	fullname = _imagedirectory.fullname(_filename);

	// find the size of the file
	struct stat	sb;
	int	rc = stat(fullname.c_str(), &sb);
	if (rc < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot stat image file '%s':  %s",
			fullname.c_str(), strerror(errno));
		throw NotFound("cannot stat image file");
	}

	// if the image has size 0, we return an empty byte sequence
	if (0 == sb.st_size) {
		return result;
	}

	// open the file to ensure that we can really read it
	int	fd = open(fullname.c_str(), O_RDONLY);
	if (fd < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot open %s: %s",
			fullname.c_str(), strerror(errno));
		throw NotFound("cannot open image file");
	}

	// read the data
	unsigned char	*buffer = new unsigned char[sb.st_size];
	if (sb.st_size != read(fd, buffer, sb.st_size)) {
		close(fd); // prevent resource leak
		std::string	msg = astro::stringprintf("could not read file %s "
			"in full length %ld", fullname.c_str(), sb.st_size);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw BadParameter(msg);
	}
	close(fd);

	// copy data from the buffer into the result
	std::copy(buffer, buffer + sb.st_size, back_inserter(result));
	delete[] buffer;

	// return the result
	return result;
}

int     ImageI::filesize(const Ice::Current& /* current */) {
	return _imagedirectory.fileSize(_filename);
}

void    ImageI::remove(const Ice::Current& /* current */) {
	_imagedirectory.remove(_filename);
}

#define	sequence_mono(pixel, size)					\
{									\
	astro::image::Image<pixel>	*imagep				\
		= dynamic_cast<astro::image::Image<pixel> *>(&*_image);	\
	if (NULL != imagep) {						\
		for (unsigned int off = 0; off < size; off++) {		\
			result.push_back((*imagep)[off]);		\
		}							\
	}								\
}

#define sequence_yuyv(pixel, size)					\
{									\
	astro::image::Image<astro::image::YUYV<pixel> >	*imagep			\
		= dynamic_cast<astro::image::Image<astro::image::YUYV<pixel> > *>(&*_image);\
	if (NULL != imagep) {						\
		for (unsigned int off = 0; off < size; off++) {		\
			result.push_back((*imagep)[off].y);		\
			result.push_back((*imagep)[off].uv);		\
		}							\
	}								\
}

#define sequence_rgb(pixel, size)					\
{									\
	astro::image::Image<astro::image::RGB<pixel> >	*imagep			\
		= dynamic_cast<astro::image::Image<astro::image::RGB<pixel> > *>(&*_image);\
	if (NULL != imagep) {						\
		for (unsigned int off = 0; off < size; off++) {		\
			result.push_back((*imagep)[off].R);		\
			result.push_back((*imagep)[off].G);		\
			result.push_back((*imagep)[off].B);		\
		}							\
	}								\
}

ByteImageI::ByteImageI(astro::image::ImageDirectory& imagedirectory,
		astro::image::ImagePtr image, const std::string& filename)
	: ImageI(imagedirectory, image, filename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "building byte image %d",
		_bytespervalue);
	if (1 != _bytespervalue) {
		std::string	msg = astro::stringprintf("cannot build byte image "
			"from %s", filename.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw BadParameter(msg);
	}
}

ByteImageI::~ByteImageI() {
}

ByteSequence	ByteImageI::getBytes(const Ice::Current& /* current */) {
	ByteSequence	result;
	unsigned int	size = _image->size().getPixels();
	sequence_mono(unsigned char, size);
	sequence_yuyv(unsigned char, size);
	sequence_rgb(unsigned char, size);
	return result;
}

ShortImageI::ShortImageI(astro::image::ImageDirectory& imagedirectory,
		astro::image::ImagePtr image, const std::string& filename)
	: ImageI(imagedirectory, image, filename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image has %d bytes per plane",
		_bytespervalue);
	if (2 != _bytespervalue) {
		std::string	msg = astro::stringprintf("cannot build "
			"short image from %s", filename.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw BadParameter(msg);
	}
}

ShortImageI::~ShortImageI() {
}

ShortSequence	ShortImageI::getShorts(const Ice::Current& /* current */) {
	ShortSequence	result;
	unsigned int	size = _image->size().getPixels();
        sequence_mono(unsigned short, size);
        sequence_yuyv(unsigned short, size);
        sequence_rgb(unsigned short, size);
	return result;
}

ImagePrx	ImageI::createProxy(const std::string& filename,
			const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create proxy for %d-size pixelvalues",
		_bytespervalue);
	return getImage(filename, _bytespervalue, current);
}

} // namespace snowstar

