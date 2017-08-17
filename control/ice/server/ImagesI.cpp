/*
 * ImagesI.cpp -- images servant implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ImagesI.h>
#include <ProxyCreator.h>
#include <ImageI.h>
#include <ImageDirectory.h>

namespace snowstar {


ImagesI::ImagesI() {
}

ImagesI::~ImagesI() {
}

ImageList	ImagesI::listImages(const Ice::Current& /* current */) {
	ImageList	result;
	astro::image::ImageDirectory	imagedirectory;
	std::list<std::string>	names = imagedirectory.fileList();
	std::copy(names.begin(), names.end(), back_inserter(result));
	return result;
}

int	ImagesI::imageSize(const std::string& name,
			const Ice::Current& /* current */) {
	astro::image::ImageDirectory	imagedirectory;
	return imagedirectory.fileSize(name);
}

int	ImagesI::imageAge(const std::string& name,
			const Ice::Current& /* current */) {
	astro::image::ImageDirectory	imagedirectory;
	return imagedirectory.fileAge(name);
}

/**
 * \brief Get an image given the name an the pixel size
 */
ImagePrx	getImage(const std::string& filename, std::type_index type,
				const Ice::Current& current) {
	// find the identity
	std::string     identity = std::string("image/") + filename;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getting image with %s pixels",
		astro::demangle(type.name()).c_str());

	// create the proxy
	if (type == typeid(unsigned char)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "unsigned char image");
		return snowstar::createProxy<ByteImagePrx>(identity, current,
			false);
	}
	if (type == typeid(unsigned short)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "unsigned short image");
		return snowstar::createProxy<ShortImagePrx>(identity, current,
			false);
	}
	if (type == typeid(unsigned int)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "unsigned int image");
		return snowstar::createProxy<IntImagePrx>(identity, current,
			false);
	}
	if (type == typeid(float)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "float image");
		return snowstar::createProxy<FloatImagePrx>(identity, current,
			false);
	}
	if (type == typeid(double)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "double image");
		return snowstar::createProxy<DoubleImagePrx>(identity, current,
			false);
	}

	std::string	msg = astro::stringprintf("unsupported pixel type:"
		" %s", astro::demangle(type.name()).c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw BadParameter(msg);
}

ImagePrx	getImage(const std::string& filename,
				const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get image named %s", filename.c_str());
	// find the number bytes per pixel
	astro::image::ImageDirectory	imagedirectory;
	std::type_index	type = imagedirectory.pixelType(filename);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "pixel type: %s",
		astro::demangle(typeid(type).name()).c_str());

	// create the proxy
	return getImage(filename, type, current);
}

} // namespace snowtar

