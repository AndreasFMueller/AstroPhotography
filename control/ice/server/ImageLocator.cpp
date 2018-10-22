/*
 * ImageLocator.cpp -- ImageLocator implementation
 *
 * (c) 2014 Prof DR Andreas Mueller, Hochschule Rapperswil
 */
#include <ImageLocator.h>
#include <ImageI.h>
#include <AstroDebug.h>
#include <AstroFilterfunc.h>
#include <ImageDirectory.h>

namespace snowstar {

/**
 * \brief Constructor for an ImageLocator
 */
ImageLocator::ImageLocator() : EvictorBase(5) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image locator created");
}

/**
 * \brief Destructor for the ImageLocator
 */
ImageLocator::~ImageLocator() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy the image locator");
}

/**
 * \brief locate an image
 *
 * This method creates an image servant of the correct pixel type
 */
Ice::ObjectPtr	ImageLocator::add(const Ice::Current& current,
					Ice::LocalObjectPtr& /* cookie */) {
	std::string	name = current.id.name;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get image %s", name.c_str());

	// have to create a new servant
	astro::image::ImageDirectory	_imagedirectory;
	Ice::ObjectPtr	ptr;
	if (!_imagedirectory.isFile(name)) {
		std::string	msg = astro::stringprintf(
			"image file %s not found", name.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw NotFound(msg);
	}
	astro::image::ImagePtr	image = _imagedirectory.getImagePtr(name);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %s image with %s pixels",
		image->size().toString().c_str(),
		astro::demangle(image->pixel_type().name()).c_str());

	// find build the image proxy matching the pixel type
	std::type_index	type = image->pixel_type();
	if ((type == typeid(unsigned char))
		|| (type == typeid(astro::image::YUYV<unsigned char>))
		|| (type == typeid(astro::image::RGB<unsigned char>))) {
		ptr = new ByteImageI(image, name);
	}
	if ((type == typeid(unsigned short))
		|| (type == typeid(astro::image::YUYV<unsigned short>))
		|| (type == typeid(astro::image::RGB<unsigned short>))) {
		ptr = new ShortImageI(image, name);
	}
	if ((type == typeid(unsigned int))
		|| (type == typeid(astro::image::YUYV<unsigned int>))
		|| (type == typeid(astro::image::RGB<unsigned int>))) {
		ptr = new IntImageI(image, name);
	}
	if ((type == typeid(float))
		|| (type == typeid(astro::image::YUYV<float>))
		|| (type == typeid(astro::image::RGB<float>))) {
		ptr = new FloatImageI(image, name);
	}
	if ((type == typeid(double))
		|| (type == typeid(astro::image::YUYV<double>))
		|| (type == typeid(astro::image::RGB<double>))) {
		ptr = new DoubleImageI(image, name);
	}
	if (!ptr) {
		BadParameter	exception;
		std::string	msg = astro::stringprintf("don't know how to "
			"handle type %s pixels",
			astro::demangle(type.name()).c_str());
		exception.cause = msg;
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw exception;
	}

	return ptr;
}

/**
 * \brief Evict an entry
 *
 * There is nothing special to be done to evict an object, but we
 * use this method to get some information about eviction happening
 */
void	ImageLocator::evict(const Ice::ObjectPtr& /* object */,
		const Ice::LocalObjectPtr& /* cookie */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "evict object");
}

} // namespace snowstar
