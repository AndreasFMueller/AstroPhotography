/*
 * deviceselector.cpp -- ComboBox to select devices of a certain type
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <deviceselector.h>
#include <AstroDebug.h>

DeviceSelector::DeviceSelector(QWidget *parent) : QComboBox(parent) {
}

DeviceSelector::~DeviceSelector() {

}

/**
 * \brief Get devices from the driver
 *
 * \param driver	driver to use when retrieving devices
 * \param type		type of device to retrieve
 */
void	DeviceSelector::devices(Astro::DriverModule_var& driver, 
		const Astro::DeviceLocator::device_type& type) {
	// ... ask for the locator ...
	Astro::DeviceLocator_var	locator
		= driver->getDeviceLocator();

	// ... and for a list of names this locator knows
	// about
	Astro::DeviceLocator::DeviceNameList_var	namelist
		= locator->getDevicelist(type);

	// add the device names to the combo box
	for (unsigned int i = 0; i < namelist->length(); i++) {
		CORBA::String_var devicename = namelist[i];
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"found device %s", (char *)devicename);
		addItem(QString((char *)devicename));
	}
}

/**
 * \brief retrieve all available objects of a given type from all modules
 *
 * \param modules	module reference to use to query for drivers
 * \param type		type of device to retrieve
 */
void	DeviceSelector::set(Astro::Modules_var& modules,
			const Astro::DeviceLocator::device_type& type) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getting device list");
	// Get a list of modules
	Astro::Modules::ModuleNameSequence_var	modulenames
		= modules->getModuleNames();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d modules", modulenames->length());

	// iterate through the modules
	for (unsigned int moduleid = 0; moduleid < modulenames->length();
		moduleid++) {
		CORBA::String_var	name = modulenames[moduleid];
		debug(LOG_DEBUG, DEBUG_LOG, 0, "working on module %s",
			(char *)name);

		// get a list of devicenames for this module
		Astro::DriverModule_var	driver = modules->getModule(name);

		// get the module descriptor
		Astro::Descriptor_var	descriptor = driver->getDescriptor();

		// if the module has a device locator, we can go on and...
		if (descriptor->hasDeviceLocator) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "getting locator for %s",
				(char *)name);
			devices(driver, type);
		}
	}

	// done
	debug(LOG_DEBUG, DEBUG_LOG, 0, "devices generated");
}
