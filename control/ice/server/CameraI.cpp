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

namespace snowstar {

std::string	CameraI::getName(const Ice::Current& current) {
	return _camera->name().toString();
}

int	CameraI::nCcds(const Ice::Current& current) {
	return _camera->nCcds();
}

CcdInfo	CameraI::getCcdinfo(int ccdid, const Ice::Current& current) {
	astro::camera::CcdInfo	info = _camera->getCcdInfo(ccdid);
	CcdInfo	result;
	result.name = info.name();
	result.id = info.getId();
	result.size.width = info.size().width();
	result.size.height = info.size().height();
	result.shutter = info.shutter();
	result.pixelheight = info.pixelheight();
	result.pixelwidth = info.pixelwidth();
	astro::camera::BinningSet::const_iterator	b;
	for (b = info.modes().begin(); b != info.modes().end(); b++) {
		BinningMode	mode;
		mode.x = b->getX();
		mode.y = b->getY();
		result.binningmodes.push_back(mode);
	}
	return result;
}

typedef IceUtil::Handle<CcdI>        CcdIPtr;

CcdPrx	CameraI::getCcd(int ccdid, const Ice::Current& current) {
	astro::camera::CcdPtr	ccd = _camera->getCcd(ccdid);
	std::string	name = ccd->name();

	// get adapter/communicator information
	Ice::ObjectAdapterPtr	adapter = current.adapter;
	Ice::CommunicatorPtr	ic = adapter->getCommunicator();

	// build the server
	CcdIPtr	servant = new CcdI(ccd);
	CcdPrx proxy = CcdPrx::uncheckedCast(
			adapter->add(servant, ic->stringToIdentity(name)));
	
	return proxy;
}

bool	CameraI::hasFilterWheel(const Ice::Current& current) {
	return _camera->hasFilterWheel();
}

typedef IceUtil::Handle<FilterWheelI>        FilterWheelIPtr;

FilterWheelPrx	CameraI::getFilterWheel(const Ice::Current& current) {
	astro::camera::FilterWheelPtr	filterwheel = _camera->getFilterWheel();
	std::string	name = filterwheel->name();

	// get adapter/communicator information
	Ice::ObjectAdapterPtr	adapter = current.adapter;
	Ice::CommunicatorPtr	ic = adapter->getCommunicator();

	// build the server
	FilterWheelIPtr	servant = new FilterWheelI(filterwheel);
	FilterWheelPrx proxy = FilterWheelPrx::uncheckedCast(
			adapter->add(servant, ic->stringToIdentity(name)));
	
	return proxy;
}

bool	CameraI::hasGuiderPort(const Ice::Current& current) {
	return _camera->hasGuiderPort();
}

typedef IceUtil::Handle<GuiderPortI>        GuiderPortIPtr;

GuiderPortPrx	CameraI::getGuiderPort(const Ice::Current& current) {
	astro::camera::GuiderPortPtr	guiderport = _camera->getGuiderPort();
	std::string	name = guiderport->name();

	// get adapter/communicator information
	Ice::ObjectAdapterPtr	adapter = current.adapter;
	Ice::CommunicatorPtr	ic = adapter->getCommunicator();

	// build the server
	GuiderPortIPtr	servant = new GuiderPortI(guiderport);
	GuiderPortPrx proxy = GuiderPortPrx::uncheckedCast(
			adapter->add(servant, ic->stringToIdentity(name)));
	
	return proxy;
}

} // namespace snowstar
