/*
 * CameraI.cpp
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CameraI.h>
#include <GuidePortI.h>
#include <FilterWheelI.h>
#include <CcdI.h>
#include <NameConverter.h>
#include <ProxyCreator.h>
#include <IceConversions.h>

namespace snowstar {

CameraI::CameraI(astro::camera::CameraPtr camera)
	: DeviceI(*camera), _camera(camera) {
}

CameraI::~CameraI() {
}

int	CameraI::nCcds(const Ice::Current& current) {
	CallStatistics::count(current);
	return _camera->nCcds();
}

CcdInfo	CameraI::getCcdinfo(int ccdid, const Ice::Current& current) {
	CallStatistics::count(current);
	astro::camera::CcdInfo	info = _camera->getCcdInfo(ccdid);
	return convert(info);
}

typedef IceUtil::Handle<CcdI>        CcdIPtr;

CcdPrx	CameraI::getCcd(int ccdid, const Ice::Current& current) {
	CallStatistics::count(current);
	std::string	name = _camera->getCcd(ccdid)->name();
	return CcdI::createProxy(name, current);
}

bool	CameraI::hasFilterWheel(const Ice::Current& current) {
	CallStatistics::count(current);
	return _camera->hasFilterWheel();
}

typedef IceUtil::Handle<FilterWheelI>        FilterWheelIPtr;

FilterWheelPrx	CameraI::getFilterWheel(const Ice::Current& current) {
	CallStatistics::count(current);
	std::string	name = _camera->getFilterWheel()->name();
	return FilterWheelI::createProxy(name, current);
}

bool	CameraI::hasGuidePort(const Ice::Current& current) {
	CallStatistics::count(current);
	return _camera->hasGuidePort();
}

typedef IceUtil::Handle<GuidePortI>        GuidePortIPtr;

GuidePortPrx	CameraI::getGuidePort(const Ice::Current& current) {
	CallStatistics::count(current);
	std::string	name = _camera->getGuidePort()->name();
	return GuidePortI::createProxy(name, current);
}

CameraPrx	CameraI::createProxy(const std::string& cameraname,
			const Ice::Current& current) {
	return snowstar::createProxy<CameraPrx>(cameraname, current);
}

} // namespace snowstar
