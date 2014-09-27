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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "instrument %s", name.c_str());
}

/**
 * \brief Retrieve a Devices proxy for a given server name
 */
DevicesPrx	RemoteInstrument::devices(const astro::ServerName& servername) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve remote devices from %s",
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

/**
 * \brief Retrieve adaptive optics proxy for an instrument
 */
AdaptiveOpticsPrx       RemoteInstrument::adaptiveoptics_proxy() {
	if (!has(astro::DeviceName::AdaptiveOptics)) {
		throw std::runtime_error("no adaptive optics device");
	}
	if (isLocal(astro::DeviceName::AdaptiveOptics)) {
		throw std::runtime_error("adaptive optics component is local");
	}
	InstrumentComponentPtr	aoptr
		= component(astro::DeviceName::AdaptiveOptics);

	// AO cannot be derived:
	if (aoptr->component_type() == InstrumentComponent::derived) {
		throw std::runtime_error("don't know how to derive AO");
	}

	// get the AO device for mapped or direct components
	astro::ServerName	servername = aoptr->servername();
	std::string	devicename = aoptr->devicename().toString();
	return devices(servername)->getAdaptiveOptics(devicename);
}

/**
 * \brief Retrieve a camera proxy
 */
CameraPrx               RemoteInstrument::camera_proxy() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieving camera proxy");
	if (!has(astro::DeviceName::Camera)) {
		throw std::runtime_error("no camera device");
	}
	if (isLocal(astro::DeviceName::Camera)) {
		throw std::runtime_error("camera component is local");
	}

	// get the camera ptr
	InstrumentComponentPtr	cameraptr
		= component(astro::DeviceName::Camera);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera name: %s",
		cameraptr->devicename().toString().c_str());

	// Camera cannot be derived
	if (cameraptr->component_type() == InstrumentComponent::derived) {
		throw std::runtime_error("don't know how to derive camera");
	}

	// get the camera device for mapped or direct components
	astro::ServerName	servername = cameraptr->servername();
	std::string	devicename = cameraptr->devicename().toString();
	CameraPrx	camera = devices(servername)->getCamera(devicename);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remote camera retrieved");
	return camera;
}

/**
 * \brief Retrieve a ccd proxy
 */
CcdPrx                  RemoteInstrument::ccd_proxy() {
	if (!has(astro::DeviceName::Ccd)) {
		throw std::runtime_error("no ccd device");
	}
	if (isLocal(astro::DeviceName::Ccd)) {
		throw std::runtime_error("ccd component is local");
	}

	// get the ccd component
	InstrumentComponentPtr	ccdptr = component(astro::DeviceName::Ccd);

	// direct or mapped devices
	switch (ccdptr->component_type()) {
	case InstrumentComponent::direct:
	case InstrumentComponent::mapped:
		{
		astro::ServerName	servername = ccdptr->servername();
		std::string	devicename = ccdptr->devicename().toString();
		return devices(ccdptr->servername())->getCcd(devicename);
		}
	case InstrumentComponent::derived:
		break;
	}

	// get the derived Ccd
	InstrumentComponentDerived	*from
		= dynamic_cast<InstrumentComponentDerived *>(&*ccdptr);
	if (from->derivedfrom() != astro::DeviceName::Camera) {
		throw std::runtime_error("only know how to derive from camera");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve Ccd from the camera");
	return camera_proxy()->getCcd(ccdptr->unit());
}

/**
 * \brief Retrieve a cooler proxy
 *
 * Coolers can be derived from a ccd
 */
CoolerPrx               RemoteInstrument::cooler_proxy() {
	if (!has(astro::DeviceName::Cooler)) {
		throw std::runtime_error("no cooler device");
	}
	if (isLocal(astro::DeviceName::Cooler)) {
		throw std::runtime_error("cooler component is local");
	}

	// get the cooler component
	InstrumentComponentPtr	coolerptr = component(astro::DeviceName::Cooler);

	// direct or mapped devices
	switch (coolerptr->component_type()) {
	case InstrumentComponent::direct:
	case InstrumentComponent::mapped:
		{
		astro::ServerName	servername = coolerptr->servername();
		std::string	devicename = coolerptr->devicename().toString();
		return devices(coolerptr->servername())->getCooler(devicename);
		}
	case InstrumentComponent::derived:
		break;
	}

	// get the derived Ccd
	InstrumentComponentDerived	*from
		= dynamic_cast<InstrumentComponentDerived *>(&*coolerptr);
	if (from->derivedfrom() != astro::DeviceName::Ccd) {
		throw std::runtime_error("onlny know how to derive from ccd");
	}
	return ccd_proxy()->getCooler();
}

/**
 * \brief Retrieve  Filterwheel proxy
 *
 * Filterwheels can be derived from a camera
 */
FilterWheelPrx          RemoteInstrument::filterwheel_proxy() {
	if (!has(astro::DeviceName::Filterwheel)) {
		throw std::runtime_error("no filterwheel device");
	}
	if (isLocal(astro::DeviceName::Filterwheel)) {
		throw std::runtime_error("filterwheel component is local");
	}

	// get the filterwheel ptr
	InstrumentComponentPtr	filterwheelptr
		= component(astro::DeviceName::Filterwheel);

	// Camera cannot be derived
	if (filterwheelptr->component_type() == InstrumentComponent::derived) {
		throw std::runtime_error("don't know how to derive filterwheel");
	}

	// get the camera device for mapped or direct components
	astro::ServerName	servername = filterwheelptr->servername();
	std::string	devicename = filterwheelptr->devicename().toString();
	return devices(servername)->getFilterWheel(devicename);
}

/**
 * \brief Retrieve a Focuser proxy
 */
FocuserPrx              RemoteInstrument::focuser_proxy() {
	if (!has(astro::DeviceName::Focuser)) {
		throw std::runtime_error("no focuser device");
	}
	if (isLocal(astro::DeviceName::Focuser)) {
		throw std::runtime_error("focuser component is local");
	}

	// get the focuser ptr
	InstrumentComponentPtr	focuserptr
		= component(astro::DeviceName::Focuser);

	// Camera cannot be derived
	if (focuserptr->component_type() == InstrumentComponent::derived) {
		throw std::runtime_error("don't know how to derive focuser");
	}

	// get the focuser device for mapped or direct components
	astro::ServerName	servername = focuserptr->servername();
	std::string	devicename = focuserptr->devicename().toString();
	return devices(servername)->getFocuser(devicename);
}

/**
 * \brief Retrive a guider port proxy
 *
 * guider ports can be derived from a camera
 */
GuiderPortPrx           RemoteInstrument::guiderport_proxy() {
	if (!has(astro::DeviceName::Guiderport)) {
		throw std::runtime_error("no guiderport device");
	}
	if (isLocal(astro::DeviceName::Guiderport)) {
		throw std::runtime_error("guiderport component is local");
	}

	// get the guiderport component
	InstrumentComponentPtr	gdptr = component(astro::DeviceName::Guiderport);

	// direct or mapped devices
	switch (gdptr->component_type()) {
	case InstrumentComponent::direct:
	case InstrumentComponent::mapped:
		{
		astro::ServerName	servername = gdptr->servername();
		std::string	devicename = gdptr->devicename().toString();
		return devices(gdptr->servername())->getGuiderPort(devicename);
		}
	case InstrumentComponent::derived:
		break;
	}

	// get the derived GuiderPort
	InstrumentComponentDerived	*from
		= dynamic_cast<InstrumentComponentDerived *>(&*gdptr);
	if (from->derivedfrom() != astro::DeviceName::Camera) {
		throw std::runtime_error("onlny know how to derive from camera");
	}
	return camera_proxy()->getGuiderPort();
}

/**
 * \brief Retrieve a mount proxy
 */
MountPrx                RemoteInstrument::mount_proxy() {
	if (!has(astro::DeviceName::Mount)) {
		throw std::runtime_error("no mount device");
	}
	if (isLocal(astro::DeviceName::Mount)) {
		throw std::runtime_error("mount component is local");
	}

	// get the mount ptr
	InstrumentComponentPtr	mountptr = component(astro::DeviceName::Mount);

	// Camera cannot be derived
	if (mountptr->component_type() == InstrumentComponent::derived) {
		throw std::runtime_error("don't know how to derive mount");
	}

	// get the focuser device for mapped or direct components
	astro::ServerName	servername = mountptr->servername();
	std::string	devicename = mountptr->devicename().toString();
	return devices(servername)->getMount(devicename);
}

} // namespace snowstar
