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
	// device related stuff
	sequence<string>	DeviceNameList;

	enum devicetype { DevAO, DevCAMERA, DevCCD,
		DevCOOLER, DevFILTERWHEEL, DevFOCUSER,
		DevGUIDERPORT, DevMODULE };
	/**
	 * \brief Device Locator interface within a module
	 */
	interface Devices {
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
		Camera*		getCamera(string name) throws NotFound;
		Ccd*		getCcd(string name) throws NotFound;
		GuiderPort*	getGuiderPort(string name) throws NotFound;
		FilterWheel*	getFilterWheel(string name) throws NotFound;
		Cooler*		getCooler(string name) throws NotFound;
		Focuser*	getFocuser(string name) throws NotFound;
	};
};

