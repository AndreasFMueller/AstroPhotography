//
// module.ice -- Interface definition for modules
//
// (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
//
#include <camera.ice>

/**
 * \brief snowstar module captures all interfaces
 */
module snowstar {
	// Module related stuff
	sequence<string>	DeviceNameList;
	enum devicetype { DevAO, DevCAMERA, DevCCD,
		DevCOOLER, DevFILTERWHEEL, DevFOCUSER,
		DevGUIDERPORT, DevMODULE };
	/**
	 * \brief Device Locator interface within a module
	 */
	interface DeviceLocator {
		string	getName();
		string	getVersion();
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
		Camera		getCamera(string name) throws NotFound;
		Ccd		getCcd(string name) throws NotFound;
		GuiderPort	getGuiderPort(string name) throws NotFound;
		FilterWheel	getFilterWheel(string name) throws NotFound;
		Cooler		getCooler(string name) throws NotFound;
		Focuser		getFocuser(string name) throws NotFound;
	};

	struct Descriptor {
		string name;
		string version;
		bool hasDeviceLocator;
	};

	/**
	 * \brief Interface to driver modules
	 *
	 * A module may implement many other things but device access.
	 * To find out whether the module gives access to any devices,
	 * one should query the Descriptor and check whether the module
	 * implements a DeviceLocator. The DeviceLocator obtained from
	 * the getDeviceLocator() method can then be used to get the
	 * devices themselves.
	 */
	interface DriverModule {
		Descriptor	getDescriptor();
		string	getName();
		/**
		 * \brief Get the device locator of a module
		 */
		DeviceLocator	getDeviceLocator() throws NotImplemented;
	};

	/**
	 * \brief Repository of all device driver modules
	 *
 	 * All device driver modules are accessible through this interface.
	 */
	sequence<string> ModuleNameSequence;
	interface Modules {
		int	numberOfModules();
		ModuleNameSequence	getModuleNames();
		DriverModule	getModule(string name) throws NotFound;
	};
};

