/*
 * DarkWork.cpp -- work to be done to build a Dark image for an imager
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroImager.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <AstroCalibration.h>
#include <AstroIO.h>

namespace astro {
namespace camera {

//////////////////////////////////////////////////////////////////////
// Implementation of the DarkWork class
//////////////////////////////////////////////////////////////////////

/**
 * \brief Construct a new DarkWork object
 *
 * By default, 10 exposures at 1 second are made
 */
DarkWork::DarkWork(CcdPtr ccd) : _ccd(ccd) {
	_exposuretime = 1.0;
	_imagecount = 10;
}

/**
 * \brief Call the end callback if present
 */
void	DarkWork::end() {
	if (_endCallback) {
		(*_endCallback)(CallbackDataPtr());
	}
}

/**
 * \brief Do the work of getting a dark image
 */
void	DarkWork::main(astro::thread::Thread<DarkWork>& thread) {
	ImagePtr	darkimage = common(thread);
	end();
}

/**
 * \brief Common work for all the dark building threads
 */
ImagePtr	DarkWork::common(astro::thread::ThreadBase& /* thread */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "DarkWork main function starts");
	// first check that all the settings are ok
	if ((_exposuretime <= 0) || (_imagecount <= 0)) {
		std::string	msg = stringprintf("bad parameters for "
			"DarkWork: exposuretime = %.3f, imagecount = %d",
			_exposuretime, _imagecount);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// construct the exposure object
	Exposure	exposure(_ccd->getInfo().getFrame(),
				_exposuretime);
	exposure.purpose(Exposure::dark);
	exposure.shutter(Shutter::CLOSED);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start to build dark %s",
		exposure.toString().c_str());

	// retrieve all the images
	ImageSequence	images;
	for (int imageno = 0; imageno < _imagecount; imageno++) {
		_ccd->startExposure(exposure);
		if (!_ccd->wait()) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"exposure %d failed, aborting",  imageno);
			return ImagePtr(NULL);
		}
		ImagePtr	image = _ccd->getImage();
		images.push_back(image);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d images");

	// construct the dark image from the images retrieved
	calibration::DarkFrameFactory	darkfactory;
	_darkimage = darkfactory(images);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got an %s dark image with %s pixels",
		_darkimage->size().toString().c_str(),
		astro::demangle(_darkimage->pixel_type().name()).c_str());

	// add additional information
	exposure.addToImage(*_darkimage);
	_darkimage->setMetadata(
		astro::io::FITSKeywords::meta(std::string("IMGCOUNT"),
			(long)_imagecount));

	return _darkimage;
}

//////////////////////////////////////////////////////////////////////
// Implementation of the DarkWorkImager class
//////////////////////////////////////////////////////////////////////

/**
 * \brief main function for the 
 */
void	DarkWorkImager::main(astro::thread::Thread<DarkWorkImager>& thread) {
	// call the common method
	ImagePtr	darkimage = common(thread);
	if (!darkimage) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no dark image received");
	}

	// install the dark image in the imager
	_imager.dark(darkimage);
	_imager.darksubtract(true);
	_imager.interpolate(true);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "dark image installed");

	// call the end callback
	end();
}

} // namespace camera
} // namespace astro
