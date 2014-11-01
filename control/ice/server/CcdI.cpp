/*
 * CcdI.cpp -- ICE CCD wrapper implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CcdI.h>
#include <CoolerI.h>
#include <NameConverter.h>
#include <AstroExceptions.h>
#include <ImageI.h>
#include <ImagesI.h>
#include <ProxyCreator.h>
#include <AstroExceptions.h>
#include <IceConversions.h>

namespace snowstar {

CcdI::~CcdI() {
}

std::string	CcdI::getName(const Ice::Current& /* current */) {
	return _ccd->name();
}

CcdInfo	CcdI::getInfo(const Ice::Current& /* current */) {
	return convert(_ccd->getInfo());
}

ExposureState	CcdI::exposureStatus(const Ice::Current& /* current */) {
	return convert(_ccd->exposureStatus());
}

void	CcdI::startExposure(const Exposure& exposure,
		const Ice::Current& /* current */) {
	image.reset();
	_ccd->startExposure(convert(exposure));
	laststart = time(NULL);
}

int	CcdI::lastExposureStart(const Ice::Current& /* current */) {
	return laststart;
}

void	CcdI::cancelExposure(const Ice::Current& /* current */) {
	try {
		_ccd->cancelExposure();
	} catch (const astro::camera::BadState& badstate) {
		throw BadState(badstate.what());
	}
}

Exposure	CcdI::getExposure(const Ice::Current& /* current */) {
	return convert(_ccd->getExposure());
}

ImagePrx	CcdI::getImage(const Ice::Current& current) {
	// get the image and add it to the ImageDirectory
	if (!image) {
		try {
			image = _ccd->getImage();
		} catch (astro::camera::BadState& bsx) {
			throw BadState("no image");
		}
	}

	// save image
	std::string	filename = _imagedirectory.save(image);
	
	return snowstar::getImage(filename, _imagedirectory, current);
}

bool	CcdI::hasGain(const Ice::Current& /* current */) {
	return _ccd->hasGain();
}

bool	CcdI::hasShutter(const Ice::Current& /* current */) {
	return _ccd->hasShutter();
}

ShutterState	CcdI::getShutterState(const Ice::Current& /* current */) {
	return convert(_ccd->getShutterState());
}

void	CcdI::setShutterState(ShutterState state,
		const Ice::Current& /* current */) {
	_ccd->setShutterState(convert(state));
}

bool	CcdI::hasCooler(const Ice::Current& /* current */) {
	return _ccd->hasCooler();
}

typedef IceUtil::Handle<CoolerI>	CoolerIPtr;

CoolerPrx	CcdI::getCooler(const Ice::Current& current) {
	std::string	name = _ccd->getCooler()->name();
	return snowstar::createProxy<CoolerPrx>(name, current);
}

CcdPrx	CcdI::createProxy(const std::string& ccdname,
		const Ice::Current& current) {
	return snowstar::createProxy<CcdPrx>(ccdname, current);
}

} // namespace snowstar
