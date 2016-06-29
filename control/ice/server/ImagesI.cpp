/*
 * ImagesI.cpp -- images servant implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ImagesI.h>
#include <ProxyCreator.h>
#include <ImageI.h>

namespace snowstar {


ImagesI::ImagesI(astro::image::ImageDirectory& _imagedirectory)
	: imagedirectory(_imagedirectory) {
}

ImagesI::~ImagesI() {
}

ImageList	ImagesI::listImages(const Ice::Current& /* current */) {
	ImageList	result;
	std::list<std::string>	names = imagedirectory.fileList();
	std::copy(names.begin(), names.end(), back_inserter(result));
	return result;
}

int	ImagesI::imageSize(const std::string& name,
			const Ice::Current& /* current */) {
	return imagedirectory.fileSize(name);
}

int	ImagesI::imageAge(const std::string& name,
			const Ice::Current& /* current */) {
	return imagedirectory.fileAge(name);
}

/**
 * \brief Get an image given the name an the pixel size
 */
ImagePrx	getImage(const std::string& filename, int bytesPerPixel,
				const Ice::Current& current) {
	// find the identity
	std::string     identity = std::string("image/") + filename;

	// create the proxy
	switch (bytesPerPixel) {
	case 1:
		return snowstar::createProxy<ByteImagePrx>(identity, current,
			false);
	case 2:
		return snowstar::createProxy<ShortImagePrx>(identity, current,
			false);
	}
	std::string	msg = astro::stringprintf("unsupported pixel size:"
		" %d", bytesPerPixel);
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw BadParameter(msg);
}

ImagePrx	getImage(const std::string& filename,
			astro::image::ImageDirectory& imagedirectory,
				const Ice::Current& current) {
	// find the number bytes per pixel
	int	bytesperplane = imagedirectory.bytesPerPlane(filename);

	// create the proxy
	return getImage(filename, bytesperplane, current);
}

} // namespace snowtar

