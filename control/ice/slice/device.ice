//
// device.ice -- Interface definition for devices
//
// (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
//
#include <camera.ice>

/**
 * \brief snowstar module captures all interfaces
 */
module snowstar {

	// state of mount
	enum mountstate { MountIDLE, MountALIGNED, MountTRACKING, MountGOTO };
	enum locationtype { LocationLOCAL, LocationGPS };

	/**
	 * \brief Interface to a telescope mount
	 */
	interface Mount extends Device {
		mountstate	state();
		RaDec	getRaDec() throws DeviceException;
		AzmAlt	getAzmAlt() throws DeviceException;
		void	GotoRaDec(RaDec radecposition) throws DeviceException;
		void	GotoAzmAlt(AzmAlt azmaltposition) throws DeviceException;
		bool	telescopePositionWest() throws DeviceException;
		void	cancel() throws DeviceException;
		LongLat	getLocation() throws DeviceException;
		locationtype	getLocationSource() throws DeviceException;
		long	getTime() throws DeviceException;
		bool	hasGuideRates() throws DeviceException;
		RaDec	getGuideRates() throws DeviceException;
	};

	// device related stuff
	sequence<string>	DeviceNameList;

	enum devicetype { DevAO, DevCAMERA, DevCCD,
		DevCOOLER, DevFILTERWHEEL, DevFOCUSER,
		DevGUIDEPORT, DevMODULE, DevMOUNT };
	/**
	 * \brief Device Locator interface within a module
	 */
	interface Devices extends Statistics {
		/**
		 * \brief Retrieve a list of device names for a device type
		 *
		 * Most camera drivers just give access to camera devices.
		 * But some devices may be accessed independently of a camera,
		 * e.g. the USB filterwheel from QHYCCD, which has it's own
		 * interface independent of a camera.
		 * \param devicetype	Type of device to construct the list
		 */
		DeviceNameList	getDevicelist(devicetype type);

		/**
		 * \brief Retrieve a device of a certain type
		 */
		AdaptiveOptics*	getAdaptiveOptics(string name) throws NotFound;
		Camera*		getCamera(string name) throws NotFound;
		Ccd*		getCcd(string name) throws NotFound;
		GuidePort*	getGuidePort(string name) throws NotFound;
		FilterWheel*	getFilterWheel(string name) throws NotFound;
		Cooler*		getCooler(string name) throws NotFound;
		Focuser*	getFocuser(string name) throws NotFound;
		Mount*		getMount(string name) throws NotFound;
	};

	/**
	 * \brief Device Locator interface, locates devices in a module
	 */
	interface DeviceLocator extends Statistics {
		string	getName();
		string	getVersion();
		DeviceNameList	getDevicelist(devicetype type);
		AdaptiveOptics*	getAdaptiveOptics(string name) throws NotFound;
		Camera*		getCamera(string name) throws NotFound;
		Ccd*		getCcd(string name) throws NotFound;
		GuidePort*	getGuidePort(string name) throws NotFound;
		FilterWheel*	getFilterWheel(string name) throws NotFound;
		Cooler*		getCooler(string name) throws NotFound;
		Focuser*	getFocuser(string name) throws NotFound;
		Mount*		getMount(string name) throws NotFound;
	};

	/**
	 * \brief interface to a driver module
 	 */
	interface DriverModule extends Statistics {
		string	getName();
		string	getVersion();
		bool	hasLocator();
		DeviceLocator*	getDeviceLocator() throws NotFound;
	};

	sequence<string>	ModuleNameList;

	/**
	 * \brief get access to modules
	 */
	interface Modules extends Statistics {
		int	numberOfModules();
		ModuleNameList	getModuleNames();
		DriverModule*	getModule(string name) throws NotFound;
	};
};

