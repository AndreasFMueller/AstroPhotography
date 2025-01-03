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
 *
 * \param progname	the program name
 */
static void	usage(const std::string& progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") +  path.basename();
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] [ <server> ] help" << std::endl;
	std::cout << p << " [ options ] <server> list" << std::endl;
	std::cout << p << " [ options ] <server> MOUNT location" << std::endl;
	std::cout << p << " [ options ] <server> MOUNT altaz" << std::endl;
	std::cout << p << " [ options ] <server> MOUNT [ get ]" << std::endl;
	std::cout << p << " [ options ] <server> MOUNT set RA DEC" << std::endl;
	std::cout << p << " [ options ] <server> MOUNT cancel" << std::endl;
	std::cout << p << " [ options ] <server> MOUNT wait" << std::endl;
	std::cout << p << " [ options ] <server> MOUNT monitor" << std::endl;
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
	std::cout << " -w,--wait          wait for goto completion in the set "
		"command" << std::endl;
	std::cout << std::endl;
}

/**
 * \brief array of options
 */
static struct option    longopts[] = {
{ "debug",	no_argument,			NULL,	'd' }, /* 1 */
{ "decimal",	no_argument,			NULL,	'f' }, /* 2 */
{ "help",	no_argument,			NULL,	'h' }, /* 3 */
{ "wait",	no_argument,			NULL,	'w' }, /* 5 */
{ NULL,		0,				NULL,	0   }
};

/**
 * \brief Help command implementation
 *
 * \param progname	the name program
 */
int	command_help(const char *progname) {
	usage(progname);
	std::cout << "The snowmount command understands the following "
		"subcommands:" << std::endl;
	std::cout << std::endl;
	std::cout << "help" << std::endl;
	std::cout << "    Display this help" << std::endl;
	std::cout << std::endl;
	std::cout << "list" << std::endl;
	std::cout << "    List all mounts available from the server"
		<< std::endl;
	std::cout << std::endl;
	std::cout << "MOUNT location" << std::endl;
	std::cout << "    Get the location of the mount" << std::endl;
	std::cout << std::endl;
	std::cout << "MOUNT get" << std::endl;
	std::cout << "    Get right ascension and declination from the named "
		"mount. This command" << std::endl;
	std::cout << "    may not work if the mount has not be calibrated yet"
		<< std::endl;
	std::cout << std::endl;
	std::cout << "MOUNT set RA DEC" << std::endl;
	std::cout << "    Move the mount to the specified right ascension and "
		"declination." << std::endl;
	std::cout << "    As with the get command, it will only work if the "
		"mount has already" << std::endl;
	std::cout << "    been calibrated." << std::endl;
	std::cout << std::endl;
	std::cout << "MOUNT wait" << std::endl;
	std::cout << "    Wait for the mount to settle on the new position"
		<< std::endl;
	std::cout << std::endl;
	std::cout << "MOUNT monitor" << std::endl;
	std::cout << "    monitor state changes and position changes on this "
		"mount." << std::endl;
	std::cout << std::endl;
	return EXIT_SUCCESS;
}

/**
 * \brief Get a list of mounts
 *
 * \param devices	the devices proxy to query for mounts
 */
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
 *
 * \param mount		the mount to get information from
 */
int	command_get(MountPrx mount) {
	RaDec	radec = mount->getRaDec();
	astro::Angle	ra;	ra.hours(radec.ra);
	astro::Angle	dec;	dec.degrees(radec.dec);
	if (!decimal) {
		std::cout << ra.hms(':', 1) << " " << dec.dms(':', 0) << " ";
	} else {
		std::cout << ra.hours() << " " << dec.degrees() << " ";
	}
	std::cout << state2string(mount->state());
	std::cout << std::endl;
	return EXIT_SUCCESS;
}

/**
 * \brief Location command implementation
 *
 * \param mount		the mount to get information from
 */
int	command_location(MountPrx mount) {
	astro::LongLat	location = convert(mount->getLocation());
	astro::Angle	longitude = location.longitude();
	astro::Angle	latitude = location.latitude();
	if (!decimal) {
		std::cout << longitude.dms(':', 0) << " " << latitude.dms(':', 0) << " ";
	} else {
		std::cout << longitude.degrees() << " " << latitude.degrees() << " ";
	}
	switch (mount->getLocationSource()) {
	case LocationLOCAL:
		std::cout << "local";
		break;
	case LocationGPS:
		std::cout << "GPS";
		break;
	}
	std::cout << std::endl;
	return EXIT_SUCCESS;
}

/**
 * \brief Time command implementation
 *
 * \param mount		the mount to get time from
 */
int	command_time(MountPrx mount) {
	astro::Time	t(mount->getTime());
	std::cout << t.toString() << std::endl;
	return  EXIT_SUCCESS;
}

/**
 * \brief Get command implementation
 *
 * \param mount		the mount to get information from
 */
int	command_altaz(MountPrx mount) {
	astro::AzmAlt	azmalt = convert(mount->getAzmAlt());
	astro::Angle	azm = azmalt.azm();
	astro::Angle	alt = azmalt.alt();
	if (!decimal) {
		std::cout << azm.hms(':', 0) << " " << alt.dms(':', 0) << " ";
	} else {
		std::cout << azm.hours() << " " << alt.degrees() << " ";
	}
	std::cout << std::endl;
	return EXIT_SUCCESS;
}

/**
 * \brief Wait command implementation
 *
 * \param mount		the mount to get information from
 * \param dowait	whether or not to wait for the state to change
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
 *
 * \param mount		the mount to cancel
 */
int	command_cancel(MountPrx mount) {
	mount->cancel();
	return command_wait(mount, await_completion);
}

/**
 * \brief Set command implementation
 *
 * \param mount		the mount to set the position
 * \param radec		the coordinates to move to
 */
int	command_set(MountPrx mount, RaDec radec) {
	mount->GotoRaDec(radec);
	return command_wait(mount, await_completion);
}

/**
 * \brief A mount callback class for monitoring
 */
class MountCallbackI : public MountCallback {
	void	timestamp() {
		astro::PrecisionTime	t;
		std::cout << t.toString("%T.%.03f:  ");
	}
public:
	virtual void	statechange(mountstate newstate,
				const Ice::Current& /* current */) {
		timestamp();
		std::cout << astro::device::Mount::state2string(
			convert(newstate));
		std::cout << std::endl;
	}
	virtual void	position(const RaDec& newposition,
				const Ice::Current& /* current */) {
		timestamp();
		astro::RaDec	position = convert(newposition);
		std::cout << position.toString() << std::endl;
	}
};

void    signal_handler(int /* sig */) {
	
}

/**
 * \brief Monitor the mount
 *
 * \param mount		the mount to monitor
 */
int	command_monitor(MountPrx mount) {
	// create a callback object
	MountCallbackI	*_callback = new MountCallbackI();
	// register the callback with the mount
	Ice::ObjectPtr  callbackptr = _callback;
        Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
        CallbackAdapter adapter(ic);
        Ice::Identity	ident = adapter.add(callbackptr);
        mount->ice_getConnection()->setAdapter(adapter.adapter());
        debug(LOG_DEBUG, DEBUG_LOG, 0, "register mount callback");
        mount->registerCallback(ident);

	// install a signal handler
	signal(SIGINT, signal_handler);
	
	// wait indefinitely
	sleep(86400);
	mount->unregisterCallback(ident);
	return 0;
}

/**
 * \brief main function 
 *
 * \param argc		the number of arguments
 * \param argv		the argument vector
 */
int	main(int argc, char *argv[]) {
	debug_set_ident("snowmount");
	CommunicatorSingleton	communicator(argc, argv);
	
	int	c;
	int	longindex;
	astro::ServerName	servername;
	putenv(strdup("POSIXLY_CORRECT=1"));
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
		case 'w':
			await_completion = true;
			break;
		default:
			throw std::runtime_error("unknown option");
		}

	// next comes the command
	if (argc <= optind) {
		throw std::runtime_error("command missing");
	}
	std::string	command(argv[optind++]);

	// handle the help command
	if (command == "help") {
		return command_help(argv[0]);
	}

	servername = astro::ServerName(command);

	// next argument must be the command
	if (argc <= optind) {
		throw std::runtime_error("command missing");
	}
	command = std::string(argv[optind++]);

	if (command == "help") {
		return command_help(argv[0]);
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
	std::string	mountname = command;
	MountPrx	mount = devices->getMount(mountname);

	// if there are no more arguments, interpret it as a get command
	if (argc <= optind) {
		return command_get(mount);
	}

	// get the command
	command = std::string(argv[optind++]);

	// get command
	if (command == "get") {
		return command_get(mount);
	}
	if (command == "location") {
		return command_location(mount);
	}
	if (command == "time") {
		return command_time(mount);
	}
	if (command == "altaz") {
		return command_altaz(mount);
	}
	if (command == "cancel") {
		return command_cancel(mount);
	}
	if (command == "wait") {
		return command_wait(mount, true);
	}
	if (command == "monitor") {
		return command_monitor(mount);
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
	int	rc = astro::main_function<snowstar::app::snowmount::main>(argc,
			argv);
	CommunicatorSingleton::release();
	return rc;
}
