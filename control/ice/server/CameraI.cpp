/*
 * CameraI.cpp
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CameraI.h>
#include <GuiderPortI.h>
#include <FilterWheelI.h>
#include <CcdI.h>
#include <Ice/Communicator.h>
#include <Ice/ObjectAdapter.h>
#include <NameConverter.h>

namespace snowstar {

CameraI::CameraI(astro::camera::CameraPtr camera) : _camera(camera) {
}

CameraI::~CameraI() {
}

std::string	CameraI::getName(const Ice::Current& current) {
	return _camera->name().toString();
}

int	CameraI::nCcds(const Ice::Current& current) {
	return _camera->nCcds();
}

CcdInfo	CameraI::getCcdinfo(int ccdid, const Ice::Current& current) {
	astro::camera::CcdInfo	info = _camera->getCcdInfo(ccdid);
	return CcdI::convert(info);
}

typedef IceUtil::Handle<CcdI>        CcdIPtr;

CcdPrx	CameraI::getCcd(int ccdid, const Ice::Current& current) {
	std::string	name = NameConverter::urlencode(_camera->getCcd(ccdid)->name());

	// get adapter/communicator information
	Ice::ObjectAdapterPtr	adapter = current.adapter;
	Ice::CommunicatorPtr	ic = adapter->getCommunicator();

	// build the server
	CcdPrx proxy = CcdPrx::uncheckedCast(
			adapter->createProxy(ic->stringToIdentity(name)));
	
	return proxy;
}

bool	CameraI::hasFilterWheel(const Ice::Current& current) {
	return _camera->hasFilterWheel();
}

typedef IceUtil::Handle<FilterWheelI>        FilterWheelIPtr;

FilterWheelPrx	CameraI::getFilterWheel(const Ice::Current& current) {
	std::string	name = NameConverter::urlencode(_camera->getFilterWheel()->name());

	// get adapter/communicator information
	Ice::ObjectAdapterPtr	adapter = current.adapter;
	Ice::CommunicatorPtr	ic = adapter->getCommunicator();

	// build the server
	FilterWheelPrx proxy = FilterWheelPrx::uncheckedCast(
			adapter->createProxy(ic->stringToIdentity(name)));
	
	return proxy;
}

bool	CameraI::hasGuiderPort(const Ice::Current& current) {
	return _camera->hasGuiderPort();
}

typedef IceUtil::Handle<GuiderPortI>        GuiderPortIPtr;

GuiderPortPrx	CameraI::getGuiderPort(const Ice::Current& current) {
	std::string	name = NameConverter::urlencode(_camera->getGuiderPort()->name());

	// get adapter/communicator information
	Ice::ObjectAdapterPtr	adapter = current.adapter;
	Ice::CommunicatorPtr	ic = adapter->getCommunicator();

	// build the server
	GuiderPortPrx proxy = GuiderPortPrx::uncheckedCast(
			adapter->createProxy(ic->stringToIdentity(name)));
	
	return proxy;
}

} // namespace snowstar
