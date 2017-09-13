/*
 * RemoteInstrument.h -- remote instrument class wrapper around an Instrument
 *
 * (c) 2014 Prof Dr Andreas Mueller Hochschule Rapperswil
 */
#ifndef _RemoteInstrument_h
#define _RemoteInstrument_h

#include <AstroConfig.h>
#include <instruments.h>
#include <device.h>
#include <camera.h>
#include <guider.h>

namespace snowstar {

/**
 * \brief Extension of the instrument class to give access to remote devices
 *
 * If a component is remote, we need to access it through ICE. This class
 * adds a method that allows to find out whether a device is remote. It also
 * provides methods that return proxies for the remote devices.
 */
class RemoteInstrument {
	DevicesPrx	devices(const astro::ServerName& servername);
	InstrumentPrx	_instrument;
	std::string	_name;
public:
	const std::string	name() const { return _name; }
	unsigned int	componentCount(InstrumentComponentType type);
	RemoteInstrument(InstrumentsPrx instruments, const std::string& name);
	RemoteInstrument();
	RemoteInstrument(const RemoteInstrument& other);
	RemoteInstrument&	operator=(const RemoteInstrument& other);
	bool	has(InstrumentComponentType type, unsigned int index = 0);
	InstrumentComponent	getComponent(InstrumentComponentType type,
					unsigned int index);
	std::string	displayname(InstrumentComponentType type,
				unsigned int index,
				const std::string& defaultservicename);
	astro::ServerName	servername(InstrumentComponentType type,
					unsigned int index = 0);
	AdaptiveOpticsPrx	adaptiveoptics(unsigned int index = 0);
	CameraPrx		camera(unsigned int index = 0);
	CcdPrx			ccd(unsigned int index = 0);
	CcdPrx			guiderccd(unsigned int index = 0);
	CoolerPrx		cooler(unsigned int index = 0);
	FilterWheelPrx		filterwheel(unsigned int index = 0);
	FocuserPrx		focuser(unsigned int index = 0);
	GuidePortPrx		guideport(unsigned int index = 0);
	MountPrx		mount(unsigned int index = 0);
	GuiderPrx		guider(unsigned int ccdindex = 0,
					unsigned int guideportindex = 0,
					unsigned int aoindex = 0);
};

} // namespace snowstar

#endif /* _RemoteInstrument_h */
