/*
 * CameraI.cpp
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CameraI.h>
#include <GuiderPortI.h>
#include <FilterWheelI.h>
#include <CcdI.h>
#include <NameConverter.h>
#include <ProxyCreator.h>
#include <IceConversions.h>

namespace snowstar {

CameraI::CameraI(astro::camera::CameraPtr camera) : _camera(camera) {
}

CameraI::~CameraI() {
}

std::string	CameraI::getName(const Ice::Current& /* current */) {
	return _camera->name().toString();
}

int	CameraI::nCcds(const Ice::Current& /* current */) {
	return _camera->nCcds();
}

CcdInfo	CameraI::getCcdinfo(int ccdid, const Ice::Current& /* current */) {
	astro::camera::CcdInfo	info = _camera->getCcdInfo(ccdid);
	return convert(info);
}

typedef IceUtil::Handle<CcdI>        CcdIPtr;

CcdPrx	CameraI::getCcd(int ccdid, const Ice::Current& current) {
	std::string	name = NameConverter::urlencode(_camera->getCcd(ccdid)->name());

	return CcdI::createProxy(name, current);
}

bool	CameraI::hasFilterWheel(const Ice::Current& /* current */) {
	return _camera->hasFilterWheel();
}

typedef IceUtil::Handle<FilterWheelI>        FilterWheelIPtr;

FilterWheelPrx	CameraI::getFilterWheel(const Ice::Current& current) {
	std::string	name = NameConverter::urlencode(_camera->getFilterWheel()->name());
	return FilterWheelI::createProxy(name, current);
}

bool	CameraI::hasGuiderPort(const Ice::Current& /* current */) {
	return _camera->hasGuiderPort();
}

typedef IceUtil::Handle<GuiderPortI>        GuiderPortIPtr;

GuiderPortPrx	CameraI::getGuiderPort(const Ice::Current& current) {
	std::string	name = NameConverter::urlencode(_camera->getGuiderPort()->name());
	return GuiderPortI::createProxy(name, current);
}

CameraPrx	CameraI::createProxy(const std::string& cameraname,
			const Ice::Current& current) {
	return snowstar::createProxy<CameraPrx>(NameConverter::urlencode(cameraname), current);
}

} // namespace snowstar
