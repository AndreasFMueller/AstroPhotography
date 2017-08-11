/*
 * GuiderBase.cpp -- implementation of guider base class
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <AstroIO.h>
#include <AstroCallback.h>

namespace astro {
namespace guiding {

/**
 * \brief start an exposure
 */
void	GuiderBase::startExposure() {
	imager().startExposure(exposure());
}

/**
 * \brief get the image
 *
 * Retrieve an image from the imager. Each image is also sent to the
 * newimagecallback, if set
 */
ImagePtr	GuiderBase::getImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getImage() called");
	imager().startExposure(exposure());
	imager().wait();
	ImagePtr	image = imager().getImage();
	if (!image->hasMetadata(std::string("INSTRUME"))) {
		image->setMetadata(astro::io::FITSKeywords::meta(
			std::string("INSTRUME"), instrument()));
	}
	_mostRecentImage = image;
	callback(image);
	return image;
}

/**
 * \brief Constructor for the guider base
 */
GuiderBase::GuiderBase(const GuiderName& guidername, camera::CcdPtr ccd,
	persistence::Database database)
	: GuiderName(guidername), _imager(ccd), _database(database)  {
}

void	GuiderBase::addImageCallback(callback::CallbackPtr callback) {
	_imagecallback.insert(callback);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "now %d image callbacks",
		_imagecallback.size());
}

void	GuiderBase::addCalibrationCallback(callback::CallbackPtr callback) {
	_calibrationcallback.insert(callback);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "now %d calibration callbacks",
		_calibrationcallback.size());
}

void	GuiderBase::addProgressCallback(callback::CallbackPtr callback) {
	_progresscallback.insert(callback);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "now %d progress callbacks",
		_progresscallback.size());
}

void	GuiderBase::addTrackingCallback(callback::CallbackPtr callback) {
	_trackingcallback.insert(callback);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "now %d tracking callbacks",
		_trackingcallback.size());
}

void	GuiderBase::addCalibrationImageCallback(callback::CallbackPtr callback) {
	_calibrationimagecallback.insert(callback);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "now %d calibration image callbacks",
		_calibrationimagecallback.size());
}

void	GuiderBase::addBacklashCallback(callback::CallbackPtr callback) {
	_backlashcallback.insert(callback);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "now %d backlash callbacks",
		_trackingcallback.size());
}


void	GuiderBase::removeImageCallback(callback::CallbackPtr callback) {
	_imagecallback.erase(callback);
}

void	GuiderBase::removeCalibrationCallback(callback::CallbackPtr callback) {
	_calibrationcallback.erase(callback);
}

void	GuiderBase::removeProgressCallback(callback::CallbackPtr callback) {
	_progresscallback.erase(callback);
}

void	GuiderBase::removeTrackingCallback(callback::CallbackPtr callback) {
	_trackingcallback.erase(callback);
}

void	GuiderBase::removeCalibrationImageCallback(callback::CallbackPtr callback) {
	_calibrationimagecallback.erase(callback);
}

void	GuiderBase::removeBacklashCallback(callback::CallbackPtr callback) {
	_backlashcallback.erase(callback);
}

/**
 * \brief Callback for images
 */
void	GuiderBase::callback(image::ImagePtr image) {
	if (!image) {
		return;
	}
	callback::ImageCallbackData	*argp
		= new callback::ImageCallbackData(image);
        callback::CallbackDataPtr arg(argp);
	_imagecallback(arg);
}

/**
 * \brief Callback for tracking points
 */
void	GuiderBase::callback(const TrackingPoint& point) {
	callback::CallbackDataPtr       trackinginfo(
		new TrackingPoint(point));
	_trackingcallback(trackinginfo);
}

/**
 * \brief Callback for calibration points
 */
void	GuiderBase::callback(const CalibrationPoint& point) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration point callback");
	callback::CallbackDataPtr	data(
		new CalibrationPointCallbackData(point));
	_calibrationcallback(data);
}

/**
 * \brief Callback for calibration points
 */
void	GuiderBase::callback(const ProgressInfo& info) {
	callback::CallbackDataPtr	data(
		new ProgressInfoCallbackData(info));
	_progresscallback(data);
}

/**
 * \brief Callback for completed calibrations
 *
 * This callback informs the guider about the status of the calibration.
 * If an incomplete calibration is received, then the guider should go
 * into state idle. For complete calibrations it should go into state
 * calibrationed.
 */
void	GuiderBase::callback(const CalibrationPtr cal) {
	// don't do anything if no calibration was sent.
	if (!cal) {
		return;
	}

	// now forward the calibration to the other callbacks
	astro::callback::CallbackDataPtr	data(
                        new CalibrationCallbackData(cal));
	_calibrationcallback(data);
}

/**
 * \brief Callback for number of calibration images
 *
 * This callback informs the guider that the calibration image process
 * has acquired a new image.
 */
void	GuiderBase::callback(const astro::camera::CalibrationImageProgress& prog) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "callback(imageno = %d/%d)",
		prog.imageno, prog.imagecount);
	astro::callback::CallbackDataPtr	data(new astro::camera::CalibrationImageProgressData(prog));
	_calibrationimagecallback(data);
}

/**
 * \brief Callback for backlash measurement points
 *
 * This Callback sends backlash measurement points to monitors
 */
void	GuiderBase::callback(const BacklashPoint& point) {
	// we cannot handle the case of termination here, because the state
	// machine is not part of the base class (this should probably be
	// changed) XXX

	astro::callback::CallbackDataPtr data(new CallbackBacklashPoint(point));
	_backlashcallback(data);
}

/**
 * \brief Callback for backlash analysis results
 *
 * This callbacks informs the guider of changes in the backlash analysis
 */
void	GuiderBase::callback(const BacklashResult& result) {
	astro::callback::CallbackDataPtr	data(
		new CallbackBacklashResult(result));
	_backlashcallback(data);
}

/**
 * \brief get a good measure for the pixel size of the CCD
 *
 * This method returns the average of the pixel dimensions, this will
 * give strange values for binned cameras. Binning looks like a strange
 * idea for a guide camera anyway.
 */
double  GuiderBase::pixelsize() const {
	astro::camera::CcdInfo	info = getCcdInfo();
	float	_pixelsize = (info.pixelwidth() * exposure().mode().x()
			+ info.pixelheight() * exposure().mode().y()) / 2.;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "pixelsize: %.2fum",
		1000000 * _pixelsize);
	return _pixelsize;
}


} // namespace guiding
} // namespace astro

