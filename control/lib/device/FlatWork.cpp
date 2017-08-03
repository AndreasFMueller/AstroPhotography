/*
 * FlatWork.cpp -- work to be done to build a Flat image for an imager
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
// Implementation of the FlatWork class
//////////////////////////////////////////////////////////////////////

/**
 * \brief Construct a new FlatWork object
 *
 * By default, 10 exposures at 1 second are made
 */
FlatWork::FlatWork(CcdPtr ccd) : _ccd(ccd) {
	_exposuretime = 1.0;
	_imagecount = 10;
}

/**
 * \brief Call the end callback if present
 */
void	FlatWork::end() {
	if (_endCallback) {
		(*_endCallback)(CallbackDataPtr());
	}
}

/**
 * \brief Do the work of getting a flat image
 */
void	FlatWork::main(astro::thread::Thread<FlatWork>& thread) {
	ImagePtr	flatimage = common(thread);
	end();
}

/**
 * \brief Common work for all the flat building threads
 */
ImagePtr	FlatWork::common(astro::thread::ThreadBase& /* thread */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "FlatWork main function starts");
	// first check that all the settings are ok
	if ((_exposuretime <= 0) || (_imagecount <= 0)) {
		std::string	msg = stringprintf("bad parameters for "
			"FlatWork: exposuretime = %.3f, imagecount = %d",
			_exposuretime, _imagecount);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// construct the exposure object
	Exposure	exposure(_ccd->getInfo().getFrame(),
				_exposuretime);
	exposure.purpose(Exposure::flat);
	exposure.shutter(Shutter::CLOSED);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start to build flat %s",
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d images", _imagecount);

	// construct the flat image from the images retrieved
	calibration::FlatFrameFactory	flatfactory;
	_flatimage = flatfactory(images, _darkimage);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got an %s flat image with %s pixels",
		_flatimage->size().toString().c_str(),
		astro::demangle(_flatimage->pixel_type().name()).c_str());

	// add additional information
	exposure.addToImage(*_flatimage);
	_flatimage->setMetadata(
		astro::io::FITSKeywords::meta(std::string("IMGCOUNT"),
			(long)_imagecount));

	return _flatimage;
}

//////////////////////////////////////////////////////////////////////
// Implementation of the FlatWorkImager class
//////////////////////////////////////////////////////////////////////

/**
 * \brief main function for the 
 */
void	FlatWorkImager::main(astro::thread::Thread<FlatWorkImager>& thread) {
	// call the common method
	ImagePtr	flatimage = common(thread);
	if (!flatimage) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no flat image received");
	}

	// install the flat image in the imager
	_imager.flat(flatimage);
	_imager.flatdivide(true);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "flat image installed");

	// call the end callback
	end();
}

} // namespace camera
} // namespace astro
