/*
 * ImagesI.cpp -- images servant implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ImagesI.h>
#include <Ice/Communicator.h>
#include <Ice/ObjectAdapter.h>

namespace snowstar {


ImagesI::ImagesI(astro::image::ImageDirectory& _imagedirectory)
	: imagedirectory(_imagedirectory) {
}

ImagesI::~ImagesI() {
}

ImageList	ImagesI::listImages(const Ice::Current& current) {
	ImageList	result;
	std::list<std::string>	names = imagedirectory.fileList();
	std::copy(names.begin(), names.end(), back_inserter(result));
	return result;
}

int	ImagesI::imageSize(const std::string& name,
			const Ice::Current& current) {
	return imagedirectory.fileSize(name);
}

int	ImagesI::imageAge(const std::string& name,
			const Ice::Current& current) {
	return imagedirectory.fileAge(name);
}

ImagePrx	ImagesI::getImage(const std::string& name,
				const Ice::Current& current) {
	std::string	identity = std::string("image/") + name;
	
	// get the adapter and communicator
	Ice::ObjectAdapterPtr	adapter = current.adapter;
	Ice::CommunicatorPtr	ic = adapter->getCommunicator();

	// build the proxy for the image
	ImagePrx	proxy = ImagePrx::uncheckedCast(
		adapter->createProxy(ic->stringToIdentity(identity)));

	return proxy;
}

} // namespace snowtar

