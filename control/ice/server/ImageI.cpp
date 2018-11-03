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
#include <AstroIO.h>
#include <AstroConfig.h>
#include <IceConversions.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/Communicator.h>
#include <ProxyCreator.h>
#include <ImagesI.h>
#include <ImageDirectory.h>
#include <ImageRepo.h>

namespace snowstar {

/**
 * \brief Construct a new Image servant
 */
ImageI::ImageI(astro::image::ImagePtr image, const std::string& filename)
	: _image(image), _filename(filename), _type(typeid(unsigned short)) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"creating image servant for %s, pixel type %s",
		_filename.c_str(), _image->pixel_type().name());
	// check whether the filename contains a /, because that we want all
	// images to be in the top level
	if (std::string::npos != _filename.find('/')) {
		std::string	msg = astro::stringprintf("file '%s' has /, should "
			"be basename", _filename.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// origin
	_origin = convert(_image->origin());
	// size
	_size = convert(_image->size());
	// type	
	_type = _image->pixel_type();
	// bytes per pixel
	_bytesperpixel = _image->bytesPerPixel();
	// bytes per value
	_bytespervalue = _image->bytesPerPlane();
	// planes
	_planes = _image->planes();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image servant created for %s",
		_filename.c_str());

	// start the timer
	time(&_lastused);
}

/**
 * \brief Destroy an image servant
 */
ImageI::~ImageI() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy servant for image %s", 
		_filename.c_str());
}

/**
 * \brief Get the name of an image
 */
std::string	ImageI::name(const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request for image %s",
		_filename.c_str());
	return _filename;
}

/**
 * \brief Get the age of an image in the directory
 */
int	ImageI::age(const Ice::Current& /* current */) {
	astro::image::ImageDirectory	_imagedirectory;
	return _imagedirectory.fileAge(_filename);
}

/**
 * \brief Get the size of the image
 */
ImageSize       ImageI::size(const Ice::Current& /* current */) {
	return _size;
}

/**
 * \brief Get the origin of the image inside the ccd frame
 */
ImagePoint      ImageI::origin(const Ice::Current& /* current */) {
	return _origin;
}

/**
 * \brief Get the size of a pixel
 */
int     ImageI::bytesPerPixel(const Ice::Current& /* current */) {
	return _bytesperpixel;
}

/**
 * \brief Get the number of image planes
 */
int     ImageI::planes(const Ice::Current& /* current */) {
	return _planes;
}

/**
 * \brief Get the size of a single value in a plane
 */
int     ImageI::bytesPerValue(const Ice::Current& /* current */) {
	return _bytespervalue;
}

/**
 * \brief Find out whether there is a keyword in the metadata
 */
bool	ImageI::hasMeta(const std::string& keyword,
			const Ice::Current& /* current */) {
	return image()->hasMetadata(keyword);
}

/**
 * \brief Get the metadata for a keyword
 */
Metavalue	ImageI::getMeta(const std::string& keyword,
			const Ice::Current& /* current */) {
	if (!image()->hasMetadata(keyword)) {
		throw NotFound("keyword not found");
	}
	return convert(_image->getMetadata(keyword));
}

/**
 * \brief Set a metadata value for an image
 */
void	ImageI::setMetavalue(const Metavalue& metavalue,
			const Ice::Current& /* current */) {
	ImageMetadata	metadata;
	metadata.setMetadata(convert(metavalue));
	astro::image::ImageDirectory	_imagedirectory;
	_imagedirectory.setMetadata(_filename, metadata);
	_image.reset();
}

/**
 * \brief Set metadata for an image
 */
void	ImageI::setMetadata(const Metadata& metadata,
			const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting metadata on file %s, %d items",
		_filename.c_str(), metadata.size());
	ImageMetadata	m;
	for (auto ptr = metadata.begin(); ptr != metadata.end(); ptr++) {
		m.setMetadata(convert(*ptr));
	}
	astro::image::ImageDirectory	_imagedirectory;
	_imagedirectory.setMetadata(_filename, m);
	_image.reset();
}

/**
 * \brief Get a image in memorey
 */
astro::image::ImagePtr	ImageI::image() {
	if (!_image) {
		astro::image::ImageDirectory	_imagedirectory;
		_image = _imagedirectory.getImagePtr(_filename);
		time(&_lastused);
	}
	return _image;
}

ImageBuffer	ImageI::fileFITS() {
	ImageBuffer	result;
	result.encoding = ImageEncodingFITS;

	// get the file
	astro::image::ImageDirectory	_imagedirectory;
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
		delete[] buffer;
		std::string	msg = astro::stringprintf("could not read file %s "
			"in full length %ld", fullname.c_str(), sb.st_size);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw BadParameter(msg);
	}
	close(fd);

	// copy data from the buffer into the result
	std::copy(buffer, buffer + sb.st_size, back_inserter(result.data));
	delete[] buffer;

	// return the result
	return result;
}

ImageBuffer	ImageI::fileJPEG() {
	astro::image::ImageBuffer	buffer(image(), Format::JPEG);
	ImageBuffer	result = *convert(buffer);
	return result;
}

ImageBuffer	ImageI::filePNG() {
	astro::image::ImageBuffer	buffer(image(), Format::PNG);
	ImageBuffer	result = *convert(buffer);
	return result;
}

/**
 * \brief Retrieve the content of the FITS file
 */
ImageBuffer       ImageI::file(ImageEncoding encoding,
			const Ice::Current& /* current */) {
	switch (encoding) {
	case ImageEncodingFITS:
		return fileFITS();
	case ImageEncodingJPEG:
		return fileJPEG();
	case ImageEncodingPNG:
		return filePNG();
	}
	throw std::runtime_error("unknown encoding");
}

/**
 * \brief Get the size of an image file
 */
int     ImageI::filesize(const Ice::Current& /* current */) {
	astro::image::ImageDirectory	_imagedirectory;
	return _imagedirectory.fileSize(_filename);
}

/**
 * \brief Save an image in a repository
 */
void	ImageI::toRepository(const std::string& reponame,
		const Ice::Current& /* current */) {
	// get the repository
	astro::project::ImageRepoPtr	repo;
	try {
		repo = ImageRepo::repo(reponame);
	} catch (const std::exception& x) {
		std::string	msg = astro::stringprintf("repo %s not found: %s",
			reponame.c_str(), x.what());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw NotFound(msg);
	}

	// add the image to the repository;
	repo->save(image());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image %s saved in %s",
		_filename.c_str(), reponame.c_str());
}

/**
 * \brief Remove the image from the image directory
 *
 * Note that this method does not remove the image itself
 */
void    ImageI::remove(const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy the image file '%s'",
		_filename.c_str());
	astro::image::ImageDirectory	_imagedirectory;
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

//////////////////////////////////////////////////////////////////////
// Byte image implementation
//////////////////////////////////////////////////////////////////////
ByteImageI::ByteImageI(astro::image::ImagePtr image,
	const std::string& filename) : ImageI(image, filename) {
	std::type_index	type = image->pixel_type();
	if ((type != typeid(unsigned char))
		&& (type != typeid(astro::image::YUYV<unsigned char>))
		&& (type != typeid(astro::image::RGB<unsigned char>))) {
		std::string	msg = astro::stringprintf("cannot build byte "
			"image from %s, has %s pixel", filename.c_str());
			astro::demangle(type.name()).c_str(),
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw BadParameter(msg);
	}
}

ByteImageI::~ByteImageI() {
}

ByteSequence	ByteImageI::getBytes(const Ice::Current& /* current */) {
	ByteSequence	result;
	unsigned int	size = image()->size().getPixels();
	sequence_mono(unsigned char, size);
	sequence_yuyv(unsigned char, size);
	sequence_rgb(unsigned char, size);
	return result;
}

//////////////////////////////////////////////////////////////////////
// Short image implementation
//////////////////////////////////////////////////////////////////////
ShortImageI::ShortImageI(astro::image::ImagePtr image,
	const std::string& filename) : ImageI(image, filename) {
	std::type_index	type = image->pixel_type();
	if ((type != typeid(unsigned short))
		&& (type != typeid(astro::image::YUYV<unsigned short>))
		&& (type != typeid(astro::image::RGB<unsigned short>))) {
		std::string	msg = astro::stringprintf("cannot build "
			"short image from %s, has %s pixel", filename.c_str(),
			astro::demangle(type.name()).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw BadParameter(msg);
	}
}

ShortImageI::~ShortImageI() {
}

ShortSequence	ShortImageI::getShorts(const Ice::Current& /* current */) {
	ShortSequence	result;
	unsigned int	size = image()->size().getPixels();
        sequence_mono(unsigned short, size);
        sequence_yuyv(unsigned short, size);
        sequence_rgb(unsigned short, size);
	return result;
}

//////////////////////////////////////////////////////////////////////
// Int image implementation
//////////////////////////////////////////////////////////////////////
IntImageI::IntImageI(astro::image::ImagePtr image,
	const std::string& filename) : ImageI(image, filename) {
	std::type_index	type = image->pixel_type();
	if ((type != typeid(unsigned int))
		&& (type != typeid(astro::image::YUYV<unsigned int>))
		&& (type != typeid(astro::image::RGB<unsigned int>))) {
		std::string	msg = astro::stringprintf("cannot build "
			"int image from %s, has %s pixel", filename.c_str(),
			astro::demangle(type.name()).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw BadParameter(msg);
	}
}

IntImageI::~IntImageI() {
}

IntSequence	IntImageI::getInts(const Ice::Current& /* current */) {
	IntSequence	result;
	unsigned int	size = image()->size().getPixels();
        sequence_mono(unsigned int, size);
        sequence_yuyv(unsigned int, size);
        sequence_rgb(unsigned int, size);
	return result;
}

//////////////////////////////////////////////////////////////////////
// Float image implementation
//////////////////////////////////////////////////////////////////////
FloatImageI::FloatImageI(astro::image::ImagePtr image,
	const std::string& filename) : ImageI(image, filename) {
	std::type_index	type = image->pixel_type();
	if ((type != typeid(float))
		&& (type != typeid(astro::image::YUYV<float>))
		&& (type != typeid(astro::image::RGB<float>))) {
		std::string	msg = astro::stringprintf("cannot build "
			"float image from %s, has %s pixel", filename.c_str(),
			astro::demangle(type.name()).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw BadParameter(msg);
	}
}

FloatImageI::~FloatImageI() {
}

FloatSequence	FloatImageI::getFloats(const Ice::Current& /* current */) {
	FloatSequence	result;
	unsigned int	size = image()->size().getPixels();
        sequence_mono(float, size);
        sequence_yuyv(float, size);
        sequence_rgb(float, size);
	return result;
}

//////////////////////////////////////////////////////////////////////
// Double image implementation
//////////////////////////////////////////////////////////////////////
DoubleImageI::DoubleImageI(astro::image::ImagePtr image,
	const std::string& filename) : ImageI(image, filename) {
	std::type_index	type = image->pixel_type();
	if ((type != typeid(double)) 
		&& (type != typeid(astro::image::YUYV<double>))
		&& (type != typeid(astro::image::RGB<double>))) {
		std::string	msg = astro::stringprintf("cannot build "
			"double image from %s, has %s pixel", filename.c_str(),
			astro::demangle(type.name()).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw BadParameter(msg);
	}
}

DoubleImageI::~DoubleImageI() {
}

DoubleSequence	DoubleImageI::getDoubles(const Ice::Current& /* current */) {
	DoubleSequence	result;
	unsigned int	size = image()->size().getPixels();
        sequence_mono(double, size);
        sequence_yuyv(double, size);
        sequence_rgb(double, size);
	return result;
}

/**
 * \brief Build an image proxy
 */
ImagePrx	ImageI::createProxy(const std::string& filename,
			const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create proxy for %s pixels",
		astro::demangle(_type.name()).c_str());
	return getImage(filename, _type, current);
}

#define	EXPIRATION_INTERVAL	30

/**
 * \brief expire images to conserver memory
 *
 * The expire method lets the servant forget the image if it has not been
 * use for some time. This releases the pixel data. If clients forget to
 * release images, large memory consumption can build up in the server
 * which may break it on the small SoC systems that we use on the telescope
 */
bool	ImageI::expire() {
	if (!_image) {
		return false;
	}
	time_t	now;
	time(&now);
	if ((now - _lastused) > EXPIRATION_INTERVAL) {
		_image.reset();
		return true;
	}
	return false;
}

} // namespace snowstar

