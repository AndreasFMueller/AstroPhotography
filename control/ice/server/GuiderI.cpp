/*
 * GuiderI.cpp -- guider servern implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <GuiderI.h>
#include <GuiderIconversions.h>
#include <CcdIconversions.h>
#include <TypesI.h>
#include <CameraI.h>
#include <CcdI.h>
#include <GuiderPortI.h>
#include <ImagesI.h>
#include <CalibrationStore.h>

namespace snowstar {

GuiderI::GuiderI(astro::guiding::GuiderPtr _guider,
	astro::image::ImageDirectory& _imagedirectory,
	astro::persistence::Database _database)
	: guider(_guider), imagedirectory(_imagedirectory),
	  database(_database) {
}

GuiderI::~GuiderI() {
}

GuiderState GuiderI::getState(const Ice::Current& /* current */) {
	return convert(guider->state());
}

CameraPrx GuiderI::getCamera(const Ice::Current& current) {
	std::string	name = guider->getDescriptor().cameraname();
	return CameraI::createProxy(name, current);
}

CcdPrx GuiderI::getCcd(const Ice::Current& current) {
	std::string	ccdname = guider->ccd()->name().toString();
	return CcdI::createProxy(ccdname, current);
}

GuiderPortPrx GuiderI::getGuiderPort(const Ice::Current& current) {
	std::string	name = guider->getDescriptor().guiderportname();
	return GuiderPortI::createProxy(name, current);
}

GuiderDescriptor GuiderI::getDescriptor(const Ice::Current& /* current */) {
	return convert(guider->getDescriptor());
}

void GuiderI::setExposure(const Exposure& exposure, const Ice::Current& /* current */) {
	guider->exposure(convert(exposure));
}

Exposure GuiderI::getExposure(const Ice::Current& /* current */) {
	return convert(guider->exposure());
}

void GuiderI::setStar(const Point& point, const Ice::Current& /* current */) {
	_point = point;
}

Point GuiderI::getStar(const Ice::Current& /* current */) {
	return _point;
}

void GuiderI::useCalibration(Ice::Int calibrationid, const Ice::Current& /* current */) {
	// retrieve guider data from the database
	astro::guiding::CalibrationStore	calibrationstore(database);
	astro::guiding::GuiderCalibration	calibration
		= calibrationstore.getCalibration(calibrationid);

	// install calibration data in the guider
	guider->calibration(calibration);
}

Calibration GuiderI::getCalibration(const Ice::Current& /* current */) {
	// XXX use database to retrieve calibration
	Calibration	calibration;
	return calibration;
}

void GuiderI::startCalibration(Ice::Float focallength,
	const Ice::Current& /* current */) {
	// XXX callback stuff

	// XXX compute calibration parameters

	// XXX construct a tracker
}

Ice::Double GuiderI::calibrationProgress(const Ice::Current& /* current */) {
	return guider->calibrationProgress();
}

void GuiderI::cancelCalibration(const Ice::Current& /* current */) {
	guider->cancelCalibration();
}

bool GuiderI::waitCalibration(Ice::Double timeout, const Ice::Current& /* current */) {
	return guider->waitCalibration(timeout);
}

astro::guiding::TrackerPtr	 GuiderI::getTracker() {
	astro::camera::Exposure	exposure = guider->exposure();
	astro::Point	d = convert(_point) - exposure.frame.origin();
	astro::image::ImagePoint	trackerstar(d.x(), d.y());
	astro::image::ImageRectangle	trackerrectangle(exposure.frame.size());
	astro::guiding::TrackerPtr	tracker(
		new astro::guiding::StarTracker(trackerstar,
			trackerrectangle, 10));
	return tracker;
}

void GuiderI::startGuiding(Ice::Float guidinginterval,
		const Ice::Current& /* current */) {
	// builde a tracker
	astro::guiding::TrackerPtr	tracker = getTracker();

	// start guiding
	guider->startGuiding(tracker, guidinginterval);

	// XXX install publisher
}

Ice::Float GuiderI::getGuidingInterval(const Ice::Current& /* current */) {
	// XXX need access to interval somehow
}

void GuiderI::stopGuiding(const Ice::Current& /* current */) {
	guider->stopGuiding();

	// XXX remove publisher
}

ImagePrx GuiderI::mostRecentImage(const Ice::Current& current) {
	// retrieve image
	astro::image::ImagePtr	image = guider->mostRecentImage;
	if (!image) {
		throw NotFound("no image available");
	}

	// store image in image directory
	std::string	filename = imagedirectory.save(image);

	// return a proxy for the image
	return getImage(filename, image->bytesPerPixel(), current);
}

TrackingPoint GuiderI::mostRecentTrackingPoint(const Ice::Current& /* current */) {
	if (astro::guiding::guiding != guider->state()) {
		throw BadState("not currently guiding");
	}

	// get info from the guider
	double	lastaction;
	astro::Point	offset;
	astro::Point	activation;
	guider->lastAction(lastaction, offset, activation);
	
	// construct a tracking point
	TrackingPoint	result;
	result.timeago = lastaction;
	result.trackingoffset = convert(offset);
	result.activation = convert(activation);
	return result;
}

TrackingHistory GuiderI::getTrackingHistory(Ice::Int id,
	const Ice::Current& /* current */) {
	// XXX implementation missing
}

} // namespace snowstar
