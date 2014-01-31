/*
 * astroinfo.cpp -- display the devices offered by a server and all the
 *                  components, options and parameters of those devices
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <module.hh>
#include <includes.h>
#include <iostream>
#include <AstroDebug.h>
#include <stdexcept>
#include <NameService.h>
#include <OrbSingleton.h>
#include <stdio.h>

namespace astro {

void	display_ccdinfo(Astro::CcdInfo *ccdinfo) {
	printf("\t\t\t\t\tName: %s\n", ccdinfo->name.in());
	printf("\t\t\t\t\tSize: %ldx%ld\n",
		ccdinfo->size.width, ccdinfo->size.height);
	printf("\t\t\t\t\tShutter: %s\n", (ccdinfo->shutter) ? "YES" : "NO");
	for (int i = 0; i < ccdinfo->binningmodes.length(); i++) {
		printf("\t\t\t\t\t\t%dx%d\n",
			ccdinfo->binningmodes[i].x,
			ccdinfo->binningmodes[i].y);
	}
}

void	display_ccd(Astro::Ccd_ptr ccd) {
	printf("\t\t\t\t\tHas shutter: %s\n",
		(ccd->hasShutter()) ? "YES" : "NO");
	printf("\t\t\t\t\tHas cooler: %s\n",
		(ccd->hasCooler()) ? "YES" : "NO");
	printf("\t\t\t\t\tHas gain: %s\n",
		(ccd->hasGain()) ? "YES" : "NO");
}

void	display_camera(int id, Astro::Camera_ptr camera) {
	printf("\t\t\tCamera[%d]: %s\n", id, camera->getName());
	printf("\t\t\t\tnumber of CCDs: %d\n", camera->nCcds());
	for (int i = 0; i < (int)camera->nCcds(); i++) {
		Astro::CcdInfo	*ccdinfo = camera->getCcdinfo(i);
		printf("\t\t\t\tCCD[%d] info:\n", i);
		display_ccdinfo(ccdinfo);
		Astro::Ccd_ptr	ccd = camera->getCcd(i);
		printf("\t\t\t\tCCD[%d] device:\n", i);
		display_ccd(ccd);
	}
}

void	display_locator(Astro::DeviceLocator_var locator) {
	Astro::DeviceLocator::DeviceNameList	*namelist
		= locator->getDevicelist(Astro::DeviceLocator::DEVICE_CAMERA);
	Astro::DeviceLocator::DeviceNameList_var	namelistvar = namelist;
	printf("\t\tnumber of cameras: %d\n", namelist->length());
	for (int i = 0; i < (int)namelist->length(); i++) {
		Astro::Camera_ptr	camera
			= locator->getCamera((*namelist)[i]);
		display_camera(i, camera);
	}
}

void	display_module(Astro::DriverModule_var drivermodule) {
	Astro::Descriptor	*descriptor = drivermodule->getDescriptor();
	printf("\tname:    %s\n", (char *)descriptor->name);
	printf("\tversion: %s\n", (char *)descriptor->version);
	if (descriptor->hasDeviceLocator) {
		Astro::DeviceLocator_var	devicelocator
			= drivermodule->getDeviceLocator();
		display_locator(devicelocator);
	} else {
		printf("\tno device locator\n");
	}
	
}

int	main(int argc, char *argv[]) {
	// get an orb reference, also removes the ORB arguments from
	// the command line
	Astro::OrbSingleton	orb(argc, argv);

	// parse the command line
	int	c;
	while (EOF != (c = getopt(argc, argv, "d")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		}

	// get a reference to the naming service
	Astro::Naming::NameService	nameservice(orb);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got naming service");

	// Next we want to get a reference to the Modules object
	Astro::Naming::Names	names;
	names.push_back(Astro::Naming::Name("Astro", "context"));
	names.push_back(Astro::Naming::Name("Modules", "object"));
	CORBA::Object_var	obj = nameservice.lookup(names);

	// get a reference to the modules interface
	Astro::Modules_var	modules = Astro::Modules::_narrow(obj);
	if (CORBA::is_nil(modules)) {
		throw std::runtime_error("nil object reference");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a reference to a Modules object");

	// convert the object into a Modules object
	std::cout << "number of modules: " << modules->numberOfModules()
		<< std::endl;

	// get the list of all modules, and display it
	Astro::Modules::ModuleNameSequence_var	namelist
		= modules->getModuleNames();
	for (int i = 0; i < (int)namelist->length(); i++) {
	
		std::cout << "module[" << i << "]: " << namelist[i] << std::endl;
		Astro::DriverModule_var	drivermodule
			= modules->getModule(namelist[i]);
		display_module(drivermodule);
	}

	// that's it, we are done
	exit(EXIT_SUCCESS);
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main(argc, argv);
	} catch (std::exception& x) {
		std::cerr << "astroinfo terminated by exception: "
			<< x.what() << std::endl;
	}
}
