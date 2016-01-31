/*
 * ImageCallbackI.cpp -- implementation of generic image callback
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ImageCallbackI.h>
#include <AstroDebug.h>
#include <AstroIO.h>
#include <IceConversions.h>

namespace snowstar {

/**
 * \brief construct a callback object
 */
ImageCallbackI::ImageCallbackI(const std::string& path,
	const std::string& prefix) : _path(path), _prefix(prefix) {
	imagecount = 0;
}

/**
 * \brief Handle the stop method of the ImageCallback interface
 */
void	ImageCallbackI::stop(const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop call received");
}

/**
 * \brief Handle the update method of the ImageCallback interface
 */
void	ImageCallbackI::update(const SimpleImage& image,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image callback update: %d x %d",
		image.size.width, image.size.height);
	std::string	filename = astro::stringprintf("%s/%s%05d.fits",
				_path.c_str(), _prefix.c_str(), imagecount++);
	astro::image::ImagePtr	imageptr = convertsimple(image);

	// write the image. These images are incomplete, they have no
	// useful FITS headers, so they are certainly not precious
	astro::io::FITSout	out(filename);
	out.setPrecious(false);
	out.write(imageptr);
}

} // namespace snowstar
