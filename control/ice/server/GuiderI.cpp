/*
 * GuiderI.cpp -- guider servern implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <GuiderI.h>
#include <IceConversions.h>
#include <CameraI.h>
#include <CcdI.h>
#include <GuidePortI.h>
#include <ImagesI.h>
#include <AstroGuiding.h>
#include <AstroConfig.h>
#include "CalibrationSource.h"
#include <AstroEvent.h>
#include <ImageDirectory.h>

namespace snowstar {

/**
 * \brief Constructor for the Guider servant
 */
GuiderI::GuiderI(astro::guiding::GuiderPtr _guider,
	astro::persistence::Database _database)
	: guider(_guider), database(_database) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guider at %p", &*_guider);
	// set point to an invalid value to allow us to detect that it 
	// has not been set
	_point.x = -1;
	_point.y = -1;
	// default tracking method is star
	_method = TrackerSTAR;

	// callback stuff
	debug(LOG_DEBUG, DEBUG_LOG, 0, "installing  callbacks");

	// guider calibration callback, called for calibration points and
	// completed calibrations
	GuiderICalibrationCallback	*ccallback
		= new GuiderICalibrationCallback(*this);
	_calibrationcallback = astro::callback::CallbackPtr(ccallback);
	guider->addCalibrationCallback(_calibrationcallback);

	// image callback, called for every image taken by the imager of
	// the guider
	GuiderIImageCallback	*icallback = new GuiderIImageCallback(*this);
	_imagecallback = astro::callback::CallbackPtr(icallback);
	guider->addImageCallback(_imagecallback);

	// tracking callback, called for every tracking point processed by
	// either of the control devices of the guider
	GuiderITrackingCallback	*tcallback = new GuiderITrackingCallback(*this);
	_trackingcallback = astro::callback::CallbackPtr(tcallback);
	guider->addTrackingCallback(_trackingcallback);

	// calibration image callback, called when the calibration image
	// callback sends en update
	GuiderICalibrationImageCallback	*cicallback
		= new GuiderICalibrationImageCallback(*this);
	_calibrationimagecallback = astro::callback::CallbackPtr(cicallback);
	guider->addCalibrationImageCallback(_calibrationimagecallback);

	// Callback for backlash data
	GuiderIBacklashCallback	*blcallback
		= new GuiderIBacklashCallback(*this);
	_backlashcallback = astro::callback::CallbackPtr(blcallback);
	guider->addBacklashCallback(_backlashcallback);
}

/**
 * \brief Guider destructor
 *
 * The main purpose of the destructor is to unregister the callbacks
 * that were registered during construction.
 */
GuiderI::~GuiderI() {
	guider->removeCalibrationCallback(_calibrationcallback);
	guider->removeImageCallback(_imagecallback);
	guider->removeTrackingCallback(_trackingcallback);
	guider->removeCalibrationImageCallback(_calibrationimagecallback);
	guider->removeBacklashCallback(_backlashcallback);
}

GuiderState GuiderI::getState(const Ice::Current& /* current */) {
	return convert(guider->state());
}

CcdPrx GuiderI::getCcd(const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getting ccd");
	astro::guiding::GuiderDescriptor	descriptor
		= guider->getDescriptor();
	std::string	ccdname = guider->getDescriptor().ccd();
	return CcdI::createProxy(ccdname, current);
}

GuidePortPrx GuiderI::getGuidePort(const Ice::Current& current) {
	std::string	name = guider->getDescriptor().guideport();
	return GuidePortI::createProxy(name, current);
}

GuiderDescriptor GuiderI::getDescriptor(const Ice::Current& /* current */) {
	return convert(guider->getDescriptor());
}

void GuiderI::setExposure(const Exposure& exposure,
	const Ice::Current& /* current */) {
	astro::camera::Exposure	e = convert(exposure);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set exposure: %s",
		e.toString().c_str());
	guider->exposure(e);
}

Exposure GuiderI::getExposure(const Ice::Current& /* current */) {
	return convert(guider->exposure());
}

/**
 * \brief Set the star to track
 *
 * \param point		star point in absolute coordinates
 */
void GuiderI::setStar(const Point& point, const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new star set: %.1f,%.1f",
		point.x, point.y);
	_point = point;
}

Point GuiderI::getStar(const Ice::Current& /* current */) {
	return _point;
}

TrackerMethod	GuiderI::getTrackerMethod(const Ice::Current& /* current */) {
	return _method;
}

void	GuiderI::setTrackerMethod(TrackerMethod method,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "using method: %s",
		(method == TrackerUNDEFINED) ? "undefined" : (
			(method == TrackerSTAR) ? "star" : (
				(method == TrackerPHASE) ? "phase" : "diff")));
	_method = method;
}

/**
 * \brief build a tracker
 */
astro::guiding::TrackerPtr	 GuiderI::getTracker() {
	// first we must make sure the data we have is consistent
	astro::camera::Exposure	exposure = guider->exposure();
	if ((exposure.frame().size().width() <= 0) ||
		(exposure.frame().size().height() <= 0)) {
		// get the frame from the ccd
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using ccd frame");
		exposure.frame(guider->imager().ccd()->getInfo().getFrame());
		guider->exposure(exposure);
	}
	if ((_point.x < 0) || (_point.y < 0)) {
		astro::image::ImagePoint	c = exposure.frame().center();
		_point.x = c.x();
		_point.y = c.y();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using ccd center (%.1f,%.1f) as star",
			_point.x, _point.y);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "current point is (%.1f, %.1f)",
		_point.x, _point.y);

	switch (_method) {
	case TrackerUNDEFINED:
	case TrackerNULL:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "construct a NULL tracker");
		return guider->getNullTracker();
		break;
	case TrackerSTAR:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "construct a star tracker");
		return guider->getTracker(convert(_point));
		break;
	case TrackerPHASE:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "construct a phase tracker");
		return guider->getPhaseTracker();
		break;
	case TrackerDIFFPHASE:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "construct a diff tracker");
		return guider->getDiffPhaseTracker();
		break;
	case TrackerLAPLACE:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "construct a laplace tracker");
		return guider->getLaplaceTracker();
		break;
	case TrackerLARGE:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "construct a large tracker");
		return guider->getLargeTracker();
		break;
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "tracking method is invalid "
		"(should not happen)");
	throw BadState("tracking method");
}

/**
 * \brief Register a callback for images taken during the process
 */
void    GuiderI::registerImageMonitor(const Ice::Identity& imagecallback,
		const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "register an image callback");
	imagecallbacks.registerCallback(imagecallback, current);
}

/**
 * \brief Unregister a callback for images
 */
void    GuiderI::unregisterImageMonitor(const Ice::Identity& imagecallback,
		const Ice::Current& current) {
	imagecallbacks.unregisterCallback(imagecallback, current);
}

/**
 * \brief Set the repository name
 */
void    GuiderI::setRepositoryName(const std::string& reponame,
                                const Ice::Current& current) {
	RepositoryUser::setRepositoryName(reponame, current);
}

/**
 * \brief Get the repository name
 */
std::string     GuiderI::getRepositoryName(const Ice::Current& current) {
	return RepositoryUser::getRepositoryName(current);
}

} // namespace snowstar
