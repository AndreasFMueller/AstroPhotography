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

ImageLocator::ImageLocator() {
}

Ice::ObjectPtr	ImageLocator::locate(const Ice::Current& current,
					Ice::LocalObjectPtr& /* cookie */) {
	std::string	name = current.id.name;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get image %s", name.c_str());

	// see whether we can satisfy the request from the cache
	imagemap::iterator	i = images.find(name);
	if (i != images.end()) {
		return i->second;
	}

	// have to create a new servant
	astro::image::ImageDirectory	_imagedirectory;
	Ice::ObjectPtr	ptr;
	if (!_imagedirectory.isFile(name)) {
		throw NotFound("image file not found");
	}
	astro::image::ImagePtr	image = _imagedirectory.getImagePtr(name);

	// find out how many bytes a pixel has
	switch (astro::image::filter::bytespervalue(image)) {
	case 1:	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating byte image servant");
		ptr = new ByteImageI(image, name);
		break;
	case 2:	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating short image servant");
		ptr = new ShortImageI(image, name);
		break;
	default:
		throw BadParameter("image of unknown pixel size");
	}

	// add the servant to the cache
	images.insert(std::make_pair(name, ptr));
	return ptr;
}

void	ImageLocator::finished(const Ice::Current& /* current */,
				const Ice::ObjectPtr& /* servant */,
				const Ice::LocalObjectPtr& /* cookie */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "finished");
}

void	ImageLocator::deactivate(const std::string& category) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "deactivate: %s", category.c_str());
}

} // namespace snowstar
