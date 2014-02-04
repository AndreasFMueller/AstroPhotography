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

namespace snowstar {

GuiderI::GuiderI(astro::guiding::GuiderPtr _guider) : guider(_guider) {
}

GuiderI::~GuiderI() {
}

GuiderState GuiderI::getState(const Ice::Current& current) {
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

GuiderDescriptor GuiderI::getDescriptor(const Ice::Current& current) {
	return convert(guider->getDescriptor());
}

void GuiderI::setExposure(const Exposure& exposure, const Ice::Current& current) {
	guider->exposure(convert(exposure));
}

Exposure GuiderI::getExposure(const Ice::Current& current) {
	return convert(guider->exposure());
}

void GuiderI::setStar(const Point& point, const Ice::Current& current) {
	_point = point;
}

Point GuiderI::getStar(const Ice::Current& current) {
	return _point;
}

void GuiderI::useCalibration(Ice::Int calibrationid, const Ice::Current& current) {
	// XXX retrieve guider data from the database

	// XXX install calibration data in the gudier
}

Calibration GuiderI::getCalibration(const Ice::Current& current) {
	// XXX use database to retrieve calibration
}

void GuiderI::startCalibration(Ice::Float, const Ice::Current& current) {
	// XXX compute calibration parameters

	// XXX construct a tracker
}

Ice::Double GuiderI::calibrationProgress(const Ice::Current& current) {
	return guider->calibrationProgress();
}

void GuiderI::cancelCalibration(const Ice::Current& current) {
	guider->cancelCalibration();
}

bool GuiderI::waitCalibration(Ice::Double timeout, const Ice::Current& current) {
	return guider->waitCalibration(timeout);
}

void GuiderI::startGuiding(Ice::Float, const Ice::Current& current) {
	// XXX builde a tracker

	// XXX start guiding

	// XXX install publisher
}

Ice::Float GuiderI::getGuidingInterval(const Ice::Current& current) {
}

void GuiderI::stopGuiding(const Ice::Current& current) {
	guider->stopGuiding();

	// XXX remove publisher
}

ImagePrx GuiderI::mostRecentImage(const Ice::Current& current) {
	// XXX retrieve image
	// XXX store image in image directory
	// XXX return a proxy for the image
	return NULL;
}

TrackingPoint GuiderI::mostRecentTrackingPoint(const Ice::Current& current) {
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

TrackingHistory GuiderI::getTrackingHistory(Ice::Int id, const Ice::Current& current) {
}

} // namespace snowstar
