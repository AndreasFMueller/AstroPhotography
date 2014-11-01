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
	throw BadParameter("pixel format not supported");
}

ImagePrx	getImage(const std::string& filename,
			astro::image::ImageDirectory& imagedirectory,
				const Ice::Current& current) {
	// find the number bytes per pixel
	int	bytesperpixel = imagedirectory.bytesPerPixel(filename);

	// create the proxy
	return getImage(filename, bytesperpixel, current);
}

} // namespace snowtar

