/*
 * snowmount.cpp -- query or position mount
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <cstdlib>
#include <device.h>
#include <CommunicatorSingleton.h>
#include <CommonClientTasks.h>
#include <includes.h>
#include <AstroConfig.h>
#include <IceConversions.h>

using namespace snowstar;

namespace snowstar {
namespace app {
namespace snowmount {

bool	await_completion = false;
bool	decimal = false;

/**
 * \brief Usage function for the snowmount function
 */
static void	usage(const std::string& progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") +  path.basename();
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] help" << std::endl;
	std::cout << p << " [ options ] list" << std::endl;
	std::cout << p << " [ options ] get MOUNT" << std::endl;
	std::cout << p << " [ options ] set MOUNT RA DEC" << std::endl;
	std::cout << p << " [ options ] cancel MOUNT" << std::endl;
	std::cout << p << " [ options ] wait MOUNT" << std::endl;
	std::cout << std::endl;
	std::cout << "get help about the snowmount command, list mounts, get "
		"right ascension from" << std::endl;
	std::cout << "the mount, or move the mount to the given coordinates."
		<< std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << std::endl;
	std::cout << " -d,--debug         increase debug level" << std::endl;
	std::cout << " -f,--decimal       display angles as decimal numbers "
		"instead of the" << std::endl;
	std::cout << "                    DD:MM:SS.sss format" << std::endl;
	std::cout << " -h,--help          display this help message"
		<< std::endl;
	std::cout << " -w,--wait          wait for goto completion"
		<< std::endl;
	std::cout << std::endl;
}

/**
 * \brief array of options
 */
static struct option    longopts[] = {
{ "debug",	no_argument,			NULL,	'd' }, /* 1 */
{ "decimal",	no_argument,			NULL,	'f' }, /* 2 */
{ "help",	no_argument,			NULL,	'h' }, /* 3 */
{ "server",	required_argument,		NULL,	's' }, /* 4 */
{ "wait",	no_argument,			NULL,	'w' }, /* 5 */
{ NULL,		0,				NULL,	0   }
};

/**
 * \brief Help command implementation
 */
int	command_help() {
	std::cout << "The snowmount command understands the ollowing "
		"subcommands:" << std::endl;
	std::cout << std::endl;
	std::cout << "help" << std::endl;
	std::cout << "    Display this help" << std::endl;
	std::cout << std::endl;
	std::cout << "list" << std::endl;
	std::cout << "    List all mounts available from the server"
		<< std::endl;
	std::cout << std::endl;
	std::cout << "get MOUNT" << std::endl;
	std::cout << "    Get right ascension and declination from the named "
		"mount. This command" << std::endl;
	std::cout << "    may not work if the mount has not be calibrated yet"
		<< std::endl;
	std::cout << std::endl;
	std::cout << "set MOUNT RA DEC" << std::endl;
	std::cout << "    Move the mount to the specified right ascension and "
		"declination." << std::endl;
	std::cout << "    As with the get command, it will only work if the "
		"mount has already" << std::endl;
	std::cout << "    been calibrated." << std::endl;
	std::cout << std::endl;
	return EXIT_SUCCESS;
}

int	command_list(DevicesPrx devices) {
	DeviceNameList	list = devices->getDevicelist(DevMOUNT);
	std::for_each(list.begin(), list.end(), 
		[](const std::string& name) {
			std::cout << name << std::endl;
		}
	);
	return EXIT_SUCCESS;
}

/**
 * \brief Get command implementation
 */
int	command_get(MountPrx mount) {
	RaDec	radec = mount->getRaDec();
	astro::Angle	ra;	ra.hours(radec.ra);
	astro::Angle	dec;	dec.degrees(radec.dec);
	if (decimal) {
		std::cout << ra.hms() << " " << dec.dms() << " ";
	} else {
		std::cout << ra.hours() << " " << dec.degrees() << " ";
	}
	std::cout << state2string(mount->state());
	std::cout << std::endl;
	return EXIT_SUCCESS;
}

/**
 * \brief Wait command implementation
 */
int	command_wait(MountPrx mount, bool dowait) {
	if (dowait) {
		mountstate	s = mount->state();
		while (s == MountGOTO) {
			sleep(1);
			s = mount->state();
		}
	}
	return command_get(mount);
}

/**
 * \brief Cancel command implementation
 */
int	command_cancel(MountPrx mount) {
	mount->cancel();
	return command_wait(mount, await_completion);
}

/**
 * \brief Set command implementation
 */
int	command_set(MountPrx mount, RaDec radec) {
	mount->GotoRaDec(radec);
	return command_wait(mount, await_completion);
}

/**
 * \brief main function 
 */
int	main(int argc, char *argv[]) {
	debug_set_ident("snowmount");
	CommunicatorSingleton	communicator(argc, argv);
	
	int	c;
	int	longindex;
	astro::ServerName	servername;
	putenv("POSIXLY_CORRECT=1");
	while (EOF != (c = getopt_long(argc, argv, "dhc:fw", longopts,
		&longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'f':
			decimal = true;
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 's':
			servername = astro::ServerName(optarg);
			break;
		case 'w':
			await_completion = true;
			break;
		}

	// next comes the command
	if (argc <= optind) {
		throw std::runtime_error("command missing");
	}
	std::string	command(argv[optind++]);

	// handle the help command
	if (command == "help") {
		return command_help();
	}

	// we need a remote device proxy for all other commands
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	Ice::ObjectPrx	base = ic->stringToProxy(servername.connect("Devices"));
	DevicesPrx	devices = DevicesPrx::checkedCast(base);

	// handle the list command
	if (command == "list") {
		return command_list(devices);
	}

	// for the other commands we need the mount name
	if (argc <= optind) {
		throw std::runtime_error("no mount name");
	}
	std::string	mountname(argv[optind++]);

	// get a proxy for the mount
	MountPrx	mount = devices->getMount(mountname);

	// get command
	if (command == "get") {
		return command_get(mount);
	}
	if (command == "cancel") {
		return command_cancel(mount);
	}
	if (command == "wait") {
		return command_wait(mount, true);
	}

	// two more arguments are angles
	if (command == "set") {
		if (argc < (optind + 2)) {
			throw std::runtime_error("missing angle arguments");
		}
		RaDec	radec;
		astro::Angle	ra
			= astro::Angle::hms_to_angle(argv[optind++]);
		radec.ra = ra.hours();
		astro::Angle	dec
			= astro::Angle::dms_to_angle(argv[optind++]);
		radec.dec = dec.degrees();
		return command_set(mount, radec);
	}

	// if we get here, then an unknown command was given
	throw std::runtime_error("unknown command");
}

} // namespace snowmount
} // namespace app
} // namespace snowstar

int	main(int argc, char *argv[]) {
	return astro::main_function<snowstar::app::snowmount::main>(argc, argv);
}
