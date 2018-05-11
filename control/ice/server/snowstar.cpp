/*
 * snowstar.cpp -- main program for the snow star server
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Ice/Ice.h>
#include <Ice/Properties.h>
#include <Ice/Initialize.h>
#include <cstdlib>
#include <iostream>
#include <includes.h>
#include <AstroDebug.h>
#include <DeviceServantLocator.h>
#include <ImageLocator.h>
#include <AstroConfig.h>
#include <CommunicatorSingleton.h>
#include <AstroEvent.h>
#include <grp.h>
#include <pwd.h>
#include "Server.h"
#include "Restart.h"

namespace snowstar {

static struct option	longopts[] = {
{ "base",		required_argument,	NULL,	'b' }, /*  0 */
{ "config",		required_argument,	NULL,	'c' }, /*  1 */
{ "confkeys",		required_argument,	NULL,	'C' }, /*  1 */
{ "debug",		no_argument,		NULL,	'd' }, /*  2 */
{ "database",		required_argument,	NULL,	'D' }, /*  3 */
{ "foreground",		no_argument,		NULL,	'f' }, /*  4 */
{ "files",		required_argument,	NULL,	'F' }, /*  5 */
{ "group",		required_argument,	NULL,	'g' }, /*  6 */
{ "help",		no_argument,		NULL,	'h' }, /*  7 */
{ "logfile",		required_argument,	NULL,	'l' }, /*  8 */
{ "lines",		required_argument,	NULL,	'N' }, /*  9 */
{ "syslog",		no_argument,		NULL,	'L' }, /* 10 */
{ "port",		required_argument,	NULL,	'p' }, /* 11 */
{ "pidfile",		required_argument,	NULL,	'P' }, /* 12 */
{ "sslport",		required_argument,	NULL,	's' }, /* 13 */
{ "name",		required_argument,	NULL,	'n' }, /* 14 */
{ "user",		required_argument,	NULL,	'u' }, /* 15 */
{ NULL,			0,			NULL,	 0  }, /* 16 */
};

static void	usage(const char *progname) {
	astro::Path	path(progname);
	std::cout << "usage: " << path.basename() << " [ options ]"
		<< std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -b,--base=<imagedir>      directory for images"
		<< std::endl;
	std::cout << " -c,--config=<configdb>    use alternative configuration "
		"database from file" << std::endl;
	std::cout << "                           configdb"
		<< std::endl;
	std::cout << " -d,--debug                enable debug mode"
		<< std::endl;
	std::cout << " -D,--database=<database>  task manager database"
		<< std::endl;
	std::cout << " -h,--help                 display this help message and "
		"exit" << std::endl;
	std::cout << " -f,--foreground           stay in foreground"
		<< std::endl;
	std::cout << " -F,--files=n              set number of log files to "
		"rotate" << std::endl;
	std::cout << " -g,--group=<group>        group to run as" << std::endl;
	std::cout << " -l,--logfile=<file>       send log to logfile named "
		"<file>" << std::endl;
	std::cout << " -L,--syslog               send log to syslog"
		<< std::endl;
	std::cout << " -N,--lines=lines          maximum number of lines per "
		"log file" << std::endl;
	std::cout << " -n,--name=<name>          define zeroconf name to use"
		<< std::endl;
	std::cout << " -p,--port=<port>          port to offer the service on"
		<< std::endl;
	std::cout << " -P,--pidfile=<file>       write the process id to "
		"<file>, and remove when exiting" << std::endl;
	std::cout << " -s,--sslport=<port>       use SSL enable port <port>"
		<< std::endl;
	std::cout << " -u,--user=<user>          user to run as" << std::endl;
}

/**
 * \brief Main function for the Snowstar server
 */
int	snowstar_main(int argc, char *argv[]) {
	debuglevel = LOG_DEBUG;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "main start");
	// copy arguments so that we can restart
	Restart	restart(argc, argv);

	// set up communicator
	CommunicatorSingleton	communicator(argc, argv);

	// default debug settings
	debugtimeprecision = 3;
	debugthreads = true;
	debug_set_ident("snowstar");
	bool	foreground = false;

	// resturn status
	int	status = EXIT_SUCCESS;

	// get properties from the command line
	Ice::PropertiesPtr	props;
	Ice::CommunicatorPtr	ic;
	try {
		props = Ice::createProperties(argc, argv);
		props->setProperty("Ice.MessageSizeMax", "65536"); // 64 MB
		props->setProperty("Ice.Plugin.IceSSL", "IceSSL:createIceSSL");
		props->setProperty("Ice.NullHandleAbort", "1");
		Ice::InitializationData	id;
		id.properties = props;
		ic = Ice::initialize(id);
	} catch (...) {
		std::cerr << "cannot initialize ICE" << std::endl;
		throw;
	}

	debuglevel = LOG_DEBUG;

	// default configuration
	std::string	databasefile("testdb.db");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "database: %s", databasefile.c_str());
	std::string	servicename("server");
	std::string	pidfilename(PIDDIR "/snowstar.pid");

	// parse the command line
	int	c;
	int	longindex;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start parsing the command line");
	while (EOF != (c = getopt_long(argc, argv, "b:Cc:dD:fghl:Ln:p:P:s:uN:F:",
		longopts, &longindex))) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found option '%c': %s",
			c, optarg);
		switch (c) {
		case 'b':
			astro::image::ImageDirectory::basedir(optarg);
			break;
		case 'C':
			astro::config::Configuration::showkeys(std::cout, true);
			exit(EXIT_SUCCESS);
			break;
		case 'c':
			debug(LOG_DEBUG, DEBUG_LOG, 0, "configuration: %s",
				optarg);
			astro::config::Configuration::set_default(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'D':
			databasefile = std::string(optarg);
			break;
		case 'f':
			foreground = true;
			break;
		case 'F':
			debugnfiles = std::stoi(optarg);
			break;
		case 'g':
			{
			struct group	*grp = getgrnam(optarg);
			if (NULL == grp) {
				debug(LOG_ERR, DEBUG_LOG, errno,
					"group %s not found", optarg);
				return EXIT_FAILURE;
			}
			debug(LOG_DEBUG, DEBUG_LOG, 0, "set gid to %d",
				grp->gr_gid);
			if (grp->gr_gid != getgid()) {
				if (setgid(grp->gr_gid)) {
					debug(LOG_ERR, DEBUG_LOG, errno,
						"cannot set gid to %d",
						grp->gr_gid);
					return EXIT_FAILURE;
				}
				if (grp->gr_gid != getgid()) {
					debug(LOG_ERR, DEBUG_LOG, 0,
						"failed to switch gid to %d",
						grp->gr_gid);
					return EXIT_FAILURE;
				}
				grp = getgrgid(getgid());
				if (NULL == grp) {
					debug(LOG_ERR, DEBUG_LOG, errno,
						"cannot get group info");
					return EXIT_FAILURE;
				}
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"group set to %s", grp->gr_name);
			}
			}
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'l':
			if (debug_file(optarg) < 0) {	
				std::cerr << "cannot open log file " << optarg
					<< ": " << strerror(errno) << std::endl;
				return EXIT_FAILURE;
			}
			break;
		case 'L':
			debug_syslog(LOG_DAEMON);
			break;
		case 'N':
			debugmaxlines = std::stoi(optarg);
			break;
		case 'n':
			astro::discover::ServiceLocation::get().servicename(std::string(optarg));
			break;
		case 'p':
			astro::discover::ServiceLocation::get().port(std::stoi(optarg));
			break;
		case 'P':
			pidfilename = std::string(optarg);
			break;
		case 's':
			astro::discover::ServiceLocation::get().sslport(std::stoi(optarg));
			break;
		case 'u':
			{
			struct passwd	*pwp = getpwnam(optarg);
			if (NULL == pwp) {
				debug(LOG_ERR, DEBUG_LOG, errno,
					"user %s not found", optarg);
				return EXIT_FAILURE;
			}
			debug(LOG_DEBUG, DEBUG_LOG, 0, "set uid to %d",
				pwp->pw_uid);
			if (getuid() != pwp->pw_uid) {
				if (setuid(pwp->pw_uid)) {
					debug(LOG_ERR, DEBUG_LOG, errno,
						"cannot set uid to %d",
						pwp->pw_uid);
					return EXIT_FAILURE;
				}
				if (pwp->pw_uid != getuid()) {
					debug(LOG_ERR, DEBUG_LOG, 0,
						"failed to switch uid to %d",
						pwp->pw_uid);
					return EXIT_FAILURE;
				}
				pwp = getpwuid(getuid());
				if (NULL == pwp) {
					debug(LOG_ERR, DEBUG_LOG, errno,
						"cannot get user info");
					return EXIT_FAILURE;
				}
				debug(LOG_DEBUG, DEBUG_LOG, 0, "user set to %s",
					pwp->pw_name);
			}
			}
			break;
		default:
			std::string	msg = astro::stringprintf("unknown "
				"option %c (0x%02x)", c, c);
			throw std::runtime_error(msg);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "command line parsed");

	// go inter the background
	if (!foreground) {
		pid_t	pid = fork();
		if (pid < 0) {
			std::cerr << "fork failed: " << strerror(errno);
			std::cerr << std::endl;
			return EXIT_FAILURE;
		}
		if (pid > 0) {
			// parent process, just exit
			return EXIT_SUCCESS;
		}
		// if get here, we are in the child process
		setsid();
		if (chdir("/") < 0) {
			std::cerr << "cannot chdir to /: " << strerror(errno);
			std::cerr<< std::endl;
			return EXIT_FAILURE;
		}
		umask(027);
	}

	{
		// by opening a new brace we ensure that the pdifile will
		// be removed when we exit from the server
		astro::PidFile	pidfile(pidfilename);

		try {
			Server	server(ic, databasefile);
			server.waitForShutdown();
			debug(LOG_DEBUG, DEBUG_LOG, 0, "server shutdown");
		} catch (const Ice::Exception& ex) {
			std::cerr << "ICE exception: " << ex.what() << std::endl;
			status = EXIT_FAILURE;
		} catch (const char *msg) {
			std::cerr << msg << std::endl;
			status = EXIT_FAILURE;
		}
		// at this point, the pid file disappears
	}

	// destroy the communicator
	if (ic) {
		ic->destroy();
	}
	astro::event(EVENT_GLOBAL, astro::events::INFO,
		astro::events::Event::SERVER, "snowstar server shutdown");

	// executing the new server
	restart.exec();

	return status;
}

} // namespace snowstar

int	main(int argc, char *argv[]) {
	return astro::main_function<snowstar::snowstar_main>(argc, argv);
}
