/*
 * snowcamera.cpp -- query or operate a camera
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <camera.h>
#include <IceConversions.h>
#include <AstroDebug.h>
#include <CommunicatorSingleton.h>
#include <CommonClientTasks.h>
#include <AstroConfig.h>
#include <includes.h>

using namespace snowstar;

namespace snowstar {
namespace app {
namespace snowcamera {

/**
 * \brief display a help message
 *
 * \param progname	the program name used in the help message
 */
static void	usage(const std::string& progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] [ <server> ] help" << std::endl;
	std::cout << p << " [ options ] <server> list" << std::endl;
	std::cout << p << " [ options ] <server> <camera>" << std::endl;
	std::cout << std::endl;
}


static struct option	longopts[] = {
{ "debug",	no_argument,			NULL,		'd' },
{ "help",	no_argument,			NULL,		'h' },
{ NULL,		0,				NULL,		 0  }
};

/**
 * \brief List all camera devices known to the server
 *
 * \param devices	the devices module to query for cameras
 */
int	command_list(DevicesPrx devices) {
	DeviceNameList  list = devices->getDevicelist(DevCAMERA);
	std::for_each(list.begin(), list.end(),
		[](const std::string& name) {
			std::cout << name << std::endl;
		}
	);
	return EXIT_SUCCESS;
}

/**
 * \brief Display information about a comera
 *
 * \param camera	the camera to query
 */
int	command_info(CameraPrx camera) {
	std::cout << "name:        " << camera->getName() << std::endl;
	std::cout << "filterwheel: "
		<< ((camera->hasFilterWheel()) ? "yes" : "no") << std::endl;
	std::cout << "guideport:   "
		<< ((camera->hasGuidePort()) ? "yes" : "no") << std::endl;
	std::cout << "ccds:        " << camera->nCcds() << std::endl;
	for (int i = 0; i < camera->nCcds(); i++) {
		auto	ccdinfo = convert(camera->getCcdinfo(i));
		std::cout << ccdinfo.toString() << std::endl;
	}
	return EXIT_SUCCESS;
}

/**
 * \brief The main function for the snowcamera program
 *
 * \param argc		the number of arguments
 * \param argv		the argumetn vector
 */
int	main(int argc, char *argv[]) {
	debug_set_ident("snowcamera");
	CommunicatorSingleton	comminicator(argc, argv);

	int	c;
	int	longindex;
	astro::ServerName	servername;
	putenv("POSIXLY_CORRECT=1");
	while (EOF != (c = getopt_long(argc, argv, "dh", longopts, &longindex)))
		switch(c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
			usage(argv[0]);
			break;
		}

	if (argc <= optind) {
		throw std::runtime_error("command missing");
	}
	std::string	command(argv[optind++]);

	if (command == "help") {
		usage(argv[0]);
		return EXIT_SUCCESS;
	}

	if (argc <= optind) {
		throw std::runtime_error("command missing");
	}
	command = std::string(argv[optind++]);

	if (command == "help") {
		usage(argv[0]);
		return EXIT_SUCCESS;
	}

	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	Ice::ObjectPrx	base = ic->stringToProxy(servername.connect("Devices"));
	DevicesPrx	devices = DevicesPrx::checkedCast(base);

	if (command == "list") {
		return command_list(devices);
	}

	std::string	cameraname = command;
	CameraPrx	camera = devices->getCamera(cameraname);
	return command_info(camera);
}

} // namepsace snowcamera
} // namespace app
} // namespace snowstar

int	main(int argc, char *argv[]) {
	int	rc = astro::main_function<snowstar::app::snowcamera::main>(argc, argv);
	CommunicatorSingleton::release();
	return rc;
}
