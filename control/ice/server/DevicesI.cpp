/*
 * DevicesI.cpp -- Device access servant implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <DevicesI.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/Communicator.h>
#include <AstroLoader.h>

namespace snowstar {

DevicesI::DevicesI(astro::module::Devices& devices)
	: _devices(devices) {
}

DevicesI::~DevicesI() {
}

DeviceNameList DevicesI::getDevicelist(devicetype type,
			const Ice::Current& current) {
	astro::module::Devices::devicelist	devicelist
		= _devices.getDevicelist(convert(type));
	return convert(devicelist);
}

Ice::ObjectPrx	DevicesI::getObject(const std::string& name,
					const Ice::Current& current) {
	Ice::ObjectAdapterPtr	adapter = current.adapter;
	Ice::CommunicatorPtr	ic = adapter->getCommunicator();
	try {
		return adapter->createProxy(ic->stringToIdentity(name));
	} catch (const astro::BadParameter& badparameter) {
		throw BadParameter(badparameter.what());
	} catch (...) {
		throw NotFound(name);
	}
}

CameraPrx	DevicesI::getCamera(const std::string& name,
					const Ice::Current& current) {
	return CameraPrx::uncheckedCast(getObject(name, current));
}

CcdPrx		DevicesI::getCcd(const std::string& name,
					const Ice::Current& current) {
	return CcdPrx::uncheckedCast(getObject(name, current));
}

GuiderPortPrx	DevicesI::getGuiderPort(const std::string& name,
					const Ice::Current& current) {
	return GuiderPortPrx::uncheckedCast(getObject(name, current));
}

FilterWheelPrx	DevicesI::getFilterWheel(const std::string& name,
					const Ice::Current& current) {
	return FilterWheelPrx::uncheckedCast(getObject(name, current));
}

CoolerPrx	DevicesI::getCooler(const std::string& name,
					const Ice::Current& current) {
	return CoolerPrx::uncheckedCast(getObject(name, current));
}

FocuserPrx	DevicesI::getFocuser(const std::string& name,
					const Ice::Current& current) {
	return FocuserPrx::uncheckedCast(getObject(name, current));
}

} // namespace snowstar
