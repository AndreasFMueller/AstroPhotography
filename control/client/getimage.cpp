/*
 * getimage.cpp -- corba client to retrieve an image from a camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "../idl/device.hh"
#include "../idl/NameService.h"
#include <includes.h>
#include <AstroDebug.h>
#include <iostream>
#include <string>
#include <stdexcept>
#include <math.h>
#include <AstroFormat.h>

namespace Astro {

void	usage(const char *progname) {
	std::cout << "usage: " << progname << " [ options ]" << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -d             increase debug level" << std::endl;
	std::cout << " -?             display this help message and exit"
		<< std::endl;
	std::cout << " -e exptime     exposure time"
		<< std::endl;
	std::cout << " -p prefix      prefix of captured image files"
		<< std::endl;
	std::cout << " -o outputdir      outputdir directory" << std::endl;
	std::cout << " -m modulename  driver modue name, type of the camera"
		<< std::endl;
	std::cout << " -C cameraid    camera number (default 0)"
		<< std::endl;
	std::cout << " -c ccdid       id of the CCD to use (default 0)"
		<< std::endl;
	std::cout << " -w width       width of image rectangle"
		<< std::endl;
	std::cout << " -h height      height of image rectangle"
		<< std::endl;
	std::cout << " -x xoffset     horizontal offset of image rectangle"
		<< std::endl;
	std::cout << " -y yoffset     vertical offset of image rectangle"
		<< std::endl;
	std::cout << " -t temp        cool the CCD to temperature <temp> in decrees Celsius"
		<< std::endl;
	std::cout << " -l             list only, lists the devices"
		<< std::endl;
}

int	main(int argc, char *argv[]) {
	// parameters
	unsigned int	cameranumber = 0;
	unsigned int	ccdid = 0;
	unsigned int	xoffset = 0;
	unsigned int	yoffset = 0;
	unsigned int	width = 0;
	unsigned int	height = 0;
	float	exposuretime = 0.01;
	const char	*outputdir = ".";
	const char	*prefix = "test";
	const char	*cameratype = "uvc";
	bool	listonly = false;
	bool	dark = false;
	double	temperature = -1;

	// initialize the ORB
	CORBA::ORB_ptr  orb = CORBA::ORB_init(argc, argv, "omniORB4");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got ORB");

	// parse command line
	int	c;
	while (EOF != (c = getopt(argc, argv, "dc:C:e:lp:o:m:h:w:x:y:?Dt:")))
		switch (c) {
		case 'D':
			dark = true;
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'e':
			exposuretime = atof(optarg);
			break;
		case 'p':
			prefix = optarg;
			break;
		case 'o':
			outputdir = optarg;
			break;
		case 'm':
			cameratype = optarg;
			break;
		case 'C':
			cameranumber = atoi(optarg);
			break;
		case 'c':
			ccdid = atoi(optarg);
			break;
		case 'l':
			listonly = true;
			break;
		case 'w':
			width = atoi(optarg);
			break;
		case 'h':
			height = atoi(optarg);
			break;
		case 'x':
			xoffset = atoi(optarg);
			break;
		case 'y':
			yoffset = atoi(optarg);
			break;
		case 't':
			temperature = atof(optarg) + 273.1;
			break;
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		}

        // get a reference to the naming service
        Astro::Naming::NameService      nameservice(orb);
        debug(LOG_DEBUG, DEBUG_LOG, 0, "got naming service");

        // Next we want to get a reference to the Modules object
        Astro::Naming::Names    names;
        names.push_back(Astro::Naming::Name("Astro", "context"));
        names.push_back(Astro::Naming::Name("Modules", "object"));
        CORBA::Object_var       obj = nameservice.lookup(names);

        // get a reference to the modules interface
        Astro::Modules_var      modules = Astro::Modules::_narrow(obj);
        if (CORBA::is_nil(modules)) {
                throw std::runtime_error("nil object reference");
        }
        debug(LOG_DEBUG, DEBUG_LOG, 0, "got a reference to a Modules object");

	// get the Module with the right name
	Astro::DriverModule_var	drivermodule = modules->getModule(cameratype);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a DriverModule reference");

	// get the device locator from the module
	Astro::DeviceLocator_ptr locator = drivermodule->getDeviceLocator();
	Astro::DeviceLocator::DeviceNameList	*namelist
		= locator->getDevicelist(Astro::DeviceLocator::DEVICE_CAMERA);
	Astro::DeviceLocator::DeviceNameList_var	namelistvar = namelist;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d device names",
		namelist->length());

	// get the camera
	Astro::Camera_ptr	camera = locator->getCamera((*namelist)[cameranumber]);
	Astro::CcdInfo	*ccdinfo = camera->getCcdinfo(ccdid);
	Astro::CcdInfo_var	ccdinfovar = ccdinfo;

	// get the CCD
	Astro::Ccd_ptr	ccd = camera->getCcd(ccdid);

	// if we have a cooler, ant the temperature is set, the we turn
	// on the usecooler flag
	bool	usecooler = false;
	if ((temperature > 0) && (ccd->hasCooler())) {
		usecooler = true;
	}

	// turn the cooler on
	Cooler_ptr	cooler;
	if (usecooler) {
		cooler = ccd->getCooler();
		cooler->setTemperature(temperature);
		cooler->setOn(true);

		// wait for the cooler to cool the chip down
		double	delta;
		do {
			sleep(1);
			double	actual = cooler->getActualTemperature();
			delta = fabs(temperature - actual);
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"set: %.1f, actual: %1.f, delta: %.1f",
				temperature, actual, delta);
		} while (delta > 1);
	}

	// prepare an exposure structure
	Astro::Exposure	exposure;
	exposure.frame.origin.x = xoffset;
	exposure.frame.origin.y = yoffset;
	exposure.frame.size.width = (width) ? width : ccdinfo->size.width;
	exposure.frame.size.height = (height) ? height: ccdinfo->size.height;
	exposure.exposuretime = exposuretime;
	exposure.gain = 1;
	exposure.limit = -1;
	exposure.shutter = (dark) ? SHUTTER_CLOSED : SHUTTER_OPEN;

	// start the exposure
	ccd->startExposure(exposure);

	// XXX wait until the state changes to exposed

	// retrieve the image
	Astro::Image_ptr	image = ccd->getImage();

	// turn of the cooler to save energy
	if (usecooler) {
		cooler->setOn(false);
	}

	// process the image
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got image of size %dx%d",
		image->size().width, image->size().height);

	return EXIT_SUCCESS;
}

} // namespace Astro

int	main(int argc, char *argv[]) {
	try {
		return Astro::main(argc, argv);
	} catch (std::exception& x) {
		std::string	msg = astro::stringprintf("%s terminated: %s",
			argv[0], x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s\n", msg.c_str());
		std::cerr << msg << std::endl;
	}
}
