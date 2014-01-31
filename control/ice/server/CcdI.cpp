/*
 * CcdI.cpp -- ICE CCD wrapper implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CcdI.h>
#include <CoolerI.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/Communicator.h>

namespace snowstar {

std::string	CcdI::getName(const Ice::Current& current) {
	return _ccd->name();
}

CcdInfo	CcdI::getInfo(const Ice::Current& current) {
	astro::camera::CcdInfo	info = _ccd->getInfo();
	CcdInfo	result;
	return result;
}

void	CcdI::startExposure(const Exposure& exposure,
		const Ice::Current& current) {
	astro::camera::Exposure	exp;
	exp.frame = astro::image::ImageRectangle(
		astro::image::ImagePoint(exposure.frame.origin.x,
			exposure.frame.origin.y),
		astro::image::ImageSize(exposure.frame.size.width,
			exposure.frame.size.height));
	exp.exposuretime = exposure.exposuretime;
	exp.gain = exposure.gain;
	exp.limit = exposure.limit;
	exp.shutter = (exposure.shutter == ShOPEN)
				? astro::camera::SHUTTER_OPEN
				: astro::camera::SHUTTER_CLOSED;
	exp.mode.setX(exposure.mode.x);
	exp.mode.setY(exposure.mode.y);
	_ccd->startExposure(exp);
	laststart = time(NULL);
}

ExposureState	CcdI::exposureStatus(const Ice::Current& current) {
	ExposureState	state;
	switch (_ccd->exposureStatus()) {
	case astro::camera::Exposure::idle:
		return IDLE;
	case astro::camera::Exposure::exposing:
		return EXPOSING;
	case astro::camera::Exposure::cancelling:
		return CANCELLING;
	case astro::camera::Exposure::exposed:
		return EXPOSED;
	}
	return state;
}

int	CcdI::lastExposureStart(const Ice::Current& current) {
	return laststart;
}

void	CcdI::cancelExposure(const Ice::Current& current) {
	try {
		_ccd->cancelExposure();
	} catch (...) {
		// XXX handle exceptions
	}
}

Exposure	CcdI::getExposure(const Ice::Current& current) {
	astro::camera::Exposure	exp = _ccd->getExposure();
	Exposure	exposure;
	exposure.frame.origin.x = exp.frame.origin().x();
	exposure.frame.origin.y = exp.frame.origin().y();
	exposure.frame.size.width = exp.frame.size().width();
	exposure.frame.size.height = exp.frame.size().height();
	exposure.exposuretime = exp.exposuretime;
	exposure.gain = exp.gain;
	exposure.limit = exp.limit;
	exposure.shutter = (exp.limit == astro::camera::SHUTTER_OPEN)
				? ShOPEN : ShCLOSED;
	exposure.mode.x = exp.mode.getX();
	exposure.mode.y = exp.mode.getY();
	return exposure;
}

ImagePrx	CcdI::getImage(const Ice::Current& current) {
	return NULL;
}

bool	CcdI::hasGain(const Ice::Current& current) {
	return _ccd->hasGain();
}

bool	CcdI::hasShutter(const Ice::Current& current) {
	return _ccd->hasShutter();
}

ShutterState	CcdI::getShutterState(const Ice::Current& current) {
	switch (_ccd->getShutterState()) {
	case astro::camera::SHUTTER_OPEN:
		return snowstar::ShOPEN;
	case astro::camera::SHUTTER_CLOSED:
		return snowstar::ShCLOSED;
	}
}

void	CcdI::setShutterState(ShutterState state, const Ice::Current& current) {
	switch (state) {
	case snowstar::ShOPEN:
		_ccd->setShutterState(astro::camera::SHUTTER_OPEN);
		break;
	case snowstar::ShCLOSED:
		_ccd->setShutterState(astro::camera::SHUTTER_CLOSED);
		break;
	}
}

bool	CcdI::hasCooler(const Ice::Current& current) {
	return _ccd->hasCooler();
}

typedef IceUtil::Handle<CoolerI>	CoolerIPtr;

CoolerPrx	CcdI::getCooler(const Ice::Current& current) {
	astro::camera::CoolerPtr	cooler = _ccd->getCooler();
	std::string	name = cooler->name();

	// get adapter/communicator information
	Ice::ObjectAdapterPtr	adapter = current.adapter;
	Ice::CommunicatorPtr	ic = adapter->getCommunicator();

	// build the server
	CoolerIPtr	servant = new CoolerI(cooler);
	CoolerPrx proxy = CoolerPrx::uncheckedCast(
			adapter->add(servant, ic->stringToIdentity(name)));
	
	return proxy;
}

} // namespace snowstar
