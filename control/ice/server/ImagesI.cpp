/*
 * ImagesI.cpp -- images servant implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ImagesI.h>
#include <ProxyCreator.h>
#include <ImageI.h>
#include <ImageDirectory.h>
#include <IceConversions.h>

namespace snowstar {


ImagesI::ImagesI() {
}

ImagesI::~ImagesI() {
}

ImageList	ImagesI::listImages(const Ice::Current& current) {
	CallStatistics::count(current);
	ImageList	result;
	astro::image::ImageDirectory	imagedirectory;
	std::list<std::string>	names = imagedirectory.fileList();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d images", names.size());
	std::copy(names.begin(), names.end(), back_inserter(result));
	return result;
}

int	ImagesI::imageSize(const std::string& name,
			const Ice::Current& current) {
	CallStatistics::count(current);
	astro::image::ImageDirectory	imagedirectory;
	return imagedirectory.fileSize(name);
}

int	ImagesI::imageAge(const std::string& name,
			const Ice::Current& current) {
	CallStatistics::count(current);
	astro::image::ImageDirectory	imagedirectory;
	return imagedirectory.fileAge(name);
}

/**
 * \brief Get an image given the name an the pixel size
 */
ImagePrx	getImage(const std::string& filename, std::type_index type,
				const Ice::Current& current) {
	CallStatistics::count(current);
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
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get image named %s", filename.c_str());
	// find the number bytes per pixel
	astro::image::ImageDirectory	imagedirectory;
	std::type_index	type = imagedirectory.pixelType(filename);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "pixel type: %s",
		astro::demangle_string(type).c_str());

	// create the proxy
	return getImage(filename, type, current);
}

void	ImagesI::remove(const std::string& filename,
			const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		astro::image::ImageDirectory	imagedirectory;
		imagedirectory.remove(filename);
	} catch (const std::exception& x) {
		NotFound	notfound;
		notfound.cause = std::string(x.what());
		throw notfound;
	}
}

std::string	ImagesI::save(const ImageFile& file,
		const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "saving file");
		astro::image::ImagePtr	image = convertfile(file);
		astro::image::ImageDirectory	imagedirectory;
		return imagedirectory.save(image);
	} catch (const std::exception& x) {
		std::string	msg = astro::stringprintf("cannot save image: "
			"%s", x.what());
		throw BadParameter(msg);
	}
}

} // namespace snowtar

