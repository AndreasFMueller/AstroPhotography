/*
 * CcdI.cpp -- ICE CCD wrapper implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CcdI.h>
#include <CoolerI.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/Communicator.h>
#include <NameConverter.h>

namespace snowstar {

CcdI::~CcdI() {
}

std::string	CcdI::getName(const Ice::Current& current) {
	return _ccd->name();
}

CcdInfo	CcdI::getInfo(const Ice::Current& current) {
	return convert(_ccd->getInfo());
}

ExposureState	CcdI::exposureStatus(const Ice::Current& current) {
	return convert(_ccd->exposureStatus());
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
	return convert(_ccd->getExposure());
}

ImagePrx	CcdI::getImage(const Ice::Current& current) {
	// get the image and add it to the ImageDirectory
	astro::image::ImagePtr	image = _ccd->getImage();
	std::string	filename = _imagedirectory.save(image);

	// create an identity
	std::string	identity = std::string("image/") + filename;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image idenity: %s", identity.c_str());

        // get the adapter and communicator
	Ice::ObjectAdapterPtr	adapter = current.adapter;
	Ice::CommunicatorPtr	ic = adapter->getCommunicator();

        // build the proxy for the image
	ImagePrx	proxy = ImagePrx::uncheckedCast(
		adapter->createProxy(ic->stringToIdentity(identity)));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "proxy returned");

        return proxy;
}

bool	CcdI::hasGain(const Ice::Current& current) {
	return _ccd->hasGain();
}

bool	CcdI::hasShutter(const Ice::Current& current) {
	return _ccd->hasShutter();
}

ShutterState	CcdI::getShutterState(const Ice::Current& current) {
	return convert(_ccd->getShutterState());
}

void	CcdI::setShutterState(ShutterState state, const Ice::Current& current) {
	_ccd->setShutterState(convert(state));
}

bool	CcdI::hasCooler(const Ice::Current& current) {
	return _ccd->hasCooler();
}

typedef IceUtil::Handle<CoolerI>	CoolerIPtr;

CoolerPrx	CcdI::getCooler(const Ice::Current& current) {
	std::string	name = NameConverter::urlencode(_ccd->getCooler()->name());

	// get adapter/communicator information
	Ice::ObjectAdapterPtr	adapter = current.adapter;
	Ice::CommunicatorPtr	ic = adapter->getCommunicator();

	// build the server
	CoolerPrx proxy = CoolerPrx::uncheckedCast(
			adapter->createProxy(ic->stringToIdentity(name)));
	
	return proxy;
}

} // namespace snowstar
