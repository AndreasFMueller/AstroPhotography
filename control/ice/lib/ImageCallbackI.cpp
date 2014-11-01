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
 * \brief Handle the upate method of the ImageCallback interface
 */
void	ImageCallbackI::update(const SimpleImage& image,
		const Ice::Current& /* current */) {
	std::string	filename = astro::stringprintf("%s/%s%05d.fits",
				_path.c_str(), _prefix.c_str(), imagecount++);
	astro::io::FITSout	out(filename);
	astro::image::ImagePtr	imageptr = convertsimple(image);
	out.write(imageptr);
}

} // namespace snowstar
