/*
 * RemoteInstrument.cpp -- class to access remote interfaces
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <RemoteInstrument.h>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <AstroFormat.h>
#include <CommunicatorSingleton.h>
#include <Ice/Ice.h>

using namespace astro::persistence;
using namespace astro::config;

namespace snowstar {

RemoteInstrument::RemoteInstrument(Database database, const std::string& name)
	: Instrument(database, name)  {
}

DevicesPrx	RemoteInstrument::devices(InstrumentComponentPtr component) {
	astro::ServerName	servername(component->servername());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve remote camera from %s",
		servername.host().c_str());

	// connect to the remote server
	std::string	connectstring
		= astro::stringprintf("Devices:default -h %s -p %hu",
			servername.host().c_str(), servername.port());

	// get a communicator ptr (singleton?)
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();

	// get the Devices object
	Ice::ObjectPrx	base = ic->stringToProxy(connectstring);
	DevicesPrx	devices = DevicesPrx::checkedCast(base);
	return devices;
}

AdaptiveOpticsPrx       RemoteInstrument::adaptiveoptics_proxy() {
	if (!has(astro::DeviceName::AdaptiveOptics)) {
		throw std::runtime_error("no adaptive optics device");
	}
	if (isLocal(astro::DeviceName::AdaptiveOptics)) {
		throw std::runtime_error("adaptive optics component is local");
	}
	InstrumentComponentPtr	c = component(astro::DeviceName::AdaptiveOptics);
	std::string	name = c->devicename().toString();
	return devices(c)->getAdaptiveOptics(name);
}

CameraPrx               RemoteInstrument::camera_proxy() {
	if (!has(astro::DeviceName::Camera)) {
		throw std::runtime_error("no camera device");
	}
	if (isLocal(astro::DeviceName::Camera)) {
		throw std::runtime_error("camera component is local");
	}
	InstrumentComponentPtr	c = component(astro::DeviceName::Camera);
	std::string	name = c->devicename().toString();
	return devices(c)->getCamera(name);
}

CcdPrx                  RemoteInstrument::ccd_proxy() {
	if (!has(astro::DeviceName::Ccd)) {
		throw std::runtime_error("no ccd device");
	}
	if (isLocal(astro::DeviceName::Ccd)) {
		throw std::runtime_error("ccd component is local");
	}
	InstrumentComponentPtr	c = component(astro::DeviceName::Ccd);
	std::string	name = c->devicename().toString();
	return devices(c)->getCcd(name);
}

CoolerPrx               RemoteInstrument::cooler_proxy() {
	if (!has(astro::DeviceName::Cooler)) {
		throw std::runtime_error("no cooler device");
	}
	if (isLocal(astro::DeviceName::Cooler)) {
		throw std::runtime_error("cooler component is local");
	}
	InstrumentComponentPtr	c = component(astro::DeviceName::Cooler);
	std::string	name = c->devicename().toString();
	return devices(c)->getCooler(name);
}

FilterWheelPrx          RemoteInstrument::filterwheel_proxy() {
	if (!has(astro::DeviceName::Filterwheel)) {
		throw std::runtime_error("no filterwheel device");
	}
	if (isLocal(astro::DeviceName::Filterwheel)) {
		throw std::runtime_error("filterwheel component is local");
	}
	InstrumentComponentPtr	c = component(astro::DeviceName::Filterwheel);
	std::string	name = c->devicename().toString();
	return devices(c)->getFilterWheel(name);
}

FocuserPrx              RemoteInstrument::focuser_proxy() {
	if (!has(astro::DeviceName::Focuser)) {
		throw std::runtime_error("no focuser device");
	}
	if (isLocal(astro::DeviceName::Focuser)) {
		throw std::runtime_error("focuser component is local");
	}
	InstrumentComponentPtr	c = component(astro::DeviceName::Focuser);
	std::string	name = c->devicename().toString();
	return devices(c)->getFocuser(name);
}

GuiderPortPrx           RemoteInstrument::guiderport_proxy() {
	if (!has(astro::DeviceName::Guiderport)) {
		throw std::runtime_error("no guiderport device");
	}
	if (isLocal(astro::DeviceName::Guiderport)) {
		throw std::runtime_error("guiderport component is local");
	}
	InstrumentComponentPtr	c = component(astro::DeviceName::Guiderport);
	std::string	name = c->devicename().toString();
	return devices(c)->getGuiderPort(name);
}

MountPrx                RemoteInstrument::mount_proxy() {
	if (!has(astro::DeviceName::Mount)) {
		throw std::runtime_error("no mount device");
	}
	if (isLocal(astro::DeviceName::Mount)) {
		throw std::runtime_error("mount component is local");
	}
	InstrumentComponentPtr	c = component(astro::DeviceName::Mount);
	std::string	name = c->devicename().toString();
	return devices(c)->getMount(name);
}

} // namespace snowstar
