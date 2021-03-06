/*
 * astromount.cpp -- get or set position on a mount
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <cstdlib>
#include <AstroDebug.h>
#include <includes.h>
#include <AstroConfig.h>
#include <AstroLoader.h>
#include <AstroDevice.h>
#include <AstroFormat.h>

using namespace astro;
using namespace astro::config;
using namespace astro::device;
using namespace astro::module;

namespace astro {
namespace app {
namespace mount {

static bool	dryrun = false;
static bool	decimal = false;
static bool	await_completion = false;

/**
 * \brief The list command
 */
int	list_command(astro::module::Devices& devices) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "list command");
	Devices::devicelist	l = devices.getDevicelist(DeviceName::Mount);
	std::for_each(l.begin(), l.end(), [](const DeviceName& name) {
			std::cout << name.toString() << std::endl;
		}
	);
	
	return EXIT_SUCCESS;
}

/**
 * \brief The help command
 */
int	help_command() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "help command");
	std::cout << "commands: help, list, get, set" << std::endl;
	std::cout << std::endl;
	std::cout << "help" << std::endl;
	std::cout << "   Display this command help message." << std::endl;
	std::cout << std::endl;
	std::cout << "list" << std::endl;
	std::cout << "   Display a list of all available mount URLs."
		<< std::endl;
	std::cout << std::endl;
	std::cout << "get MOUNT" << std::endl;
	std::cout << "    Receive current position and tracking status of "
		"mount with" << std::endl;
	std::cout << "    device name MOUNT" << std::endl;
	std::cout << std::endl;
	std::cout << "set MOUNT RA DEC" << std::endl;
	std::cout << "    Position the mount to right ascension RA and "
		"declination DEC." << std::endl;
	std::cout << "    RA has te be specified in decimal hours, and DEC in "
		"decimal degrees" << std::endl;
	std::cout << "    On most mounts this will only work if the mount has "
		"been calibrated." << std::endl;
	std::cout << std::endl;
	std::cout << "cancel MOUNT" << std::endl;
	std::cout << "    Cancel a GOTO command currently in process"
		<< std::endl;
	std::cout << std::endl;
	std::cout << "wait MOUNT" << std::endl;
	std::cout << "    Wait completion of a GOTO or cancel command."
		<< std::endl;
	std::cout << "time MOUNT" << std::endl;
	std::cout << "    Get the (GPS) time from the mount."
		<< std::endl;
	return EXIT_SUCCESS;
}

static std::string	state2string(Mount::state_type state) {
	switch (state) {
	case Mount::IDLE:
		return std::string("idle");
	case Mount::ALIGNED:
		return std::string("aligned");
	case Mount::TRACKING:
		return std::string("tracking");
	case Mount::GOTO:
		return std::string("goto");
	}
	std::string	cause = stringprintf("unknown mount state: %d",
		(int)state);
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
	throw std::runtime_error(cause);
}

/**
 * \brief Implementation of the get command
 */
int	get_command(MountPtr mount) {
	RaDec	radec = mount->getRaDec();
	if (decimal) {
		std::cout << radec.ra().hours();
		std::cout << " ";
		if (radec.dec() > Angle(M_PI)) {
			std::cout << (radec.dec() - Angle(2 * M_PI)).degrees();
		} else {
			std::cout << radec.dec().degrees();
		}
	} else {
		std::cout << radec.ra().hms();
		std::cout << " ";
		if (radec.dec() > Angle(M_PI)) {
			std::cout << (radec.dec() - Angle(2 * M_PI)).dms();
		} else {
			std::cout << radec.dec().dms();
		}
	}
	std::cout << " ";
	std::cout << state2string(mount->state());
	std::cout << " ";
	if (mount->telescopePositionWest()) {
		std::cout << "W";
	} else {
		std::cout << "E";
	}
	std::cout << std::endl;
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of wait command
 */
int	wait_command(MountPtr mount, bool dowait) {
	if (dowait) {
		Mount::state_type	state = mount->state();
		do {
			sleep(1);
			state = mount->state();
		} while (state == Mount::GOTO);
	}
	return get_command(mount);
}

/**
 * \brief Implementation of cancel command
 */
int	cancel_command(MountPtr mount) {
	mount->cancel();
	return wait_command(mount, await_completion);
}

/**
 * \brief Implementation of the set command
 */
int	set_command(MountPtr mount, const RaDec& radec) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ra = %s", radec.ra().hms().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "dec = %s", radec.dec().dms().c_str());
	if (!dryrun) {
		mount->Goto(radec);
		return wait_command(mount, await_completion);
	}
	return get_command(mount);
}

/**
 * \brief Implementation of the time command
 */
int	time_command(MountPtr mount) {
	time_t	t = mount->time();
	std::cout << stringprintf("%s", ctime(&t));
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the guidereates command
 */
int	location_command(MountPtr mount) {
	LongLat	location = mount->location();
	std::cout << location.longitude().dms().c_str();
	std::cout << " ";
	std::cout << location.latitude().dms().c_str();
	std::cout << " ";
	switch (mount->location_source()) {
	case Mount::LOCAL:	std::cout << "local"; break;
	case Mount::GPS:	std::cout << "GPS"; break;
	}
	std::cout << std::endl;
	return 0;
}

/**
 * \brief Implementation of the guiderates command
 */
int	guiderates_command(MountPtr mount) {
	if (!mount->hasGuideRates()) {
		std::cout << "mount has no guide rates" << std::endl;
		return 1;
	}
	astro::RaDec	guiderates = mount->getGuideRates();
	std::cout << "RA rate:  " << guiderates.ra().hms().c_str() << std::endl;
	std::cout << "DEC rate: " << guiderates.dec().hms().c_str() << std::endl;
	return 0;
}

/**
 * \brief Table of options for the astromount command
 */
static struct option    longopts[] = {
/* name		argument?		int*		int */
{ "config",	required_argument,	NULL,		'c' }, /* 0 */
{ "debug",	no_argument,		NULL,		'd' }, /* 1 */
{ "help",	no_argument,		NULL,		'h' }, /* 2 */
{ "dryrun",	no_argument,		NULL,		'n' }, /* 3 */
{ "decimal",	no_argument,		NULL,		'f' }, /* 4 */
{ "wait",	no_argument,		NULL,		'w' }, /* 5 */
{ NULL,		0,			NULL,		0   }
};

static void	usage(const std::string& progname) {
	std::string	prg = std::string("    ") + Path(progname).basename();
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << prg << " [ options ] help" << std::endl;
	std::cout << prg << " [ options ] list" << std::endl;
	std::cout << prg << " [ options ] get MOUNT" << std::endl;
	std::cout << prg << " [ options ] set MOUNT ra dec" << std::endl;
	std::cout << prg << " [ options ] cancel MOUNT" << std::endl;
	std::cout << prg << " [ options ] wait MOUNT" << std::endl;
	std::cout << prg << " [ options ] time MOUNT" << std::endl;
	std::cout << prg << " [ options ] location MOUNT" << std::endl;
	std::cout << prg << " [ options ] guiderates MOUNT" << std::endl;
	std::cout << std::endl;
	std::cout << "list mounts, get or set RA and DEC of a mount";
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << " -h,--help          display help message and exit"
		 << std::endl;
	std::cout << " -c,--config=<cfg>  use configuration from file <cfg>"
		<< std::endl;
	std::cout << " -d,--debug         increase debug level" << std::endl;
	std::cout << " -f,--decimal       display angles in decimal format"
		<< std::endl;
	std::cout << " -n,--dryrun        dry run, parse arguments but don't "
		"move telescope" << std::endl;
	std::cout << " -w,--wait          wait for completion of goto command"
		<< std::endl;
	std::cout << std::endl;
}

/**
 * \brief Main method
 */
int main(int argc, char *argv[]) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mount utility");
	int	c;
	int	longindex;
	putenv(strdup("POSIXLY_CORRECT=1"));
	while (EOF != (c = getopt_long(argc, argv, "c:dh?", longopts,
		&longindex))) {
		switch (c) {
		case 'c':
			Configuration::set_default(std::string(optarg));
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'n':
			dryrun = true;
			break;
		case 'f':
			decimal = true;
			break;
		case 'w':
			await_completion = true;
			break;
		case 1:
			switch (longindex) {
			}
			break;
		}
	}

	// next argument must be the command
	if (argc <= optind) {
		throw std::runtime_error("missing command argument");
	}
	std::string	command(argv[optind++]);

	// call the command specific functions
	if (command == "help") {
		return help_command();
	}

	// the other commands need a repository
	auto	repository = ModuleRepository::get();
	astro::module::Devices	devices(repository);

	// list command
	if (command == "list") {
		return list_command(devices);
	}

	// other commands need a mount url 
	if (argc <= optind) {
		throw std::runtime_error("missing mount URL");
	}
	DeviceName	mountname(argv[optind++]);
	if (!mountname.hasType(DeviceName::Mount)) {
		throw std::runtime_error("not a mount device name");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mount device name: %s",
		mountname.toString().c_str());

	// use the Devices class to get the mount associated with this name
	MountPtr	mount = devices.getMount(mountname);
	if (command == "get") {
		return get_command(mount);
	}
	if (command == "cancel") {
		return cancel_command(mount);
	}
	if (command == "wait") {
		return wait_command(mount, true);
	}
	if (command == "set") {
		if (argc < optind + 2) {
			throw std::runtime_error("two angle arguments missing");
		}
		RaDec	radec;
		radec.ra() = Angle::hms_to_angle(argv[optind++]);
		radec.dec() = Angle::dms_to_angle(argv[optind++]);
		return set_command(mount, radec);
	}
	if (command == "time") {
		return time_command(mount);
	}
	if (command == "location") {
		return location_command(mount);
	}
	if (command == "guiderates") {
		return guiderates_command(mount);
	}

	throw std::runtime_error("unknown command");
}

} // namespace mount
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::mount::main>(argc, argv);
}
