/*
 * RemoteInstrument.h -- remote instrument class wrapper around an Instrument
 *
 * (c) 2014 Prof Dr Andreas Mueller Hochschule Rapperswil
 */
#ifndef _RemoteInstrument_h
#define _RemoteInstrument_h

#include <AstroConfig.h>
#include <device.h>
#include <camera.h>

namespace snowstar {

/**
 * \brief Extension of the instrument class to give access to remote devices
 *
 * If a component is remote, we need to access it through ICE. This class
 * adds a method that allows to find out whether a device is remote. It also
 * provides methods that return proxies for the remote devices.
 */
class RemoteInstrument : public astro::config::Instrument {
	DevicesPrx	devices(const astro::ServerName& servername);
public:
	RemoteInstrument(astro::persistence::Database database,
		const std::string& name);

	AdaptiveOpticsPrx	adaptiveoptics_proxy();
	CameraPrx		camera_proxy();
	CcdPrx			ccd_proxy();
	CoolerPrx		cooler_proxy();
	FilterWheelPrx		filterwheel_proxy();
	FocuserPrx		focuser_proxy();
	GuiderPortPrx		guiderport_proxy();
	MountPrx		mount_proxy();
};

} // namespace snowstar

#endif /* _RemoteInstrument_h */
