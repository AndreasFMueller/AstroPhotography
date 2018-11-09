/*
 * snowgateway.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <CommunicatorSingleton.h>
#include <includes.h>
#include <AstroConfig.h>
#include <types.h>
#include <IceConversions.h>
#include <CommonClientTasks.h>
#include <AstroFormat.h>
#include <getopt.h>

namespace snowstar {
namespace app {
namespace gateway {

bool	completed = false;
std::string	instrument;
std::string	urlstring;
std::string	execstring;
astro::RaDec	telescope;
astro::LongLat	location;

void	signal_handler(int /* sig */) {
	completed = true;
}

class StatusUpdateMonitorI : public StatusUpdateMonitor {
	std::string	_urlstring;
	std::string	_execstring;
public:
	const std::string&	urlstring() const { return _urlstring; }
	void	urlstring(const std::string& u) { _urlstring = u; }
	const std::string&	execstring() const { return _execstring; }
	void	execstring(const std::string& e) { _execstring = e; }
	StatusUpdateMonitorI() {
	}
	virtual ~StatusUpdateMonitorI() {
	}
	virtual void	update(const StatusUpdate& statusupdate,
			const Ice::Current& /* current */) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "update received: %s",
			convert(statusupdate).toString().c_str());
		if (_urlstring.size() > 0) {
			astro::URL	url(_urlstring);
			astro::PostData	pd = convert(statusupdate);
			int	rc = url.post(pd);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "POST return code: %d",
				rc);
		}
		if (_execstring.size() > 0) {
			std::string	cmd
				= astro::stringprintf("%s %.5f %.5f %.5f %.5f",
				_execstring.c_str(),
				statusupdate.telescope.ra,
				statusupdate.telescope.dec,
				statusupdate.observatory.longitude,
				statusupdate.observatory.latitude);
			int	rc = system(cmd.c_str());
			debug(LOG_DEBUG, DEBUG_LOG, 0, "cmd '%s' rc=%d",
				cmd.c_str(), rc);
			if (rc != 0) {
				debug(LOG_ERR, DEBUG_LOG, 0,
					"command %s returns error %d",
					cmd.c_str(), rc);
			}
		}
	}
	virtual void	stop(const Ice::Current& /* current */) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "stop received");
		completed = true;
	}
};

void	usage(const char *progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] help" << std::endl;
	std::cout << p << " [ options ] <service> help" << std::endl;
	std::cout << p << " [ options ] <service> send" << std::endl;
	std::cout << p << " [ options ] <service> monitor" << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -d,--debug                increase debug level";
	std::cout << std::endl;
	std::cout << "  -e,--exec=<prog>          program to execute for "
		"each udpate";
	std::cout << std::endl;
	std::cout << "  -h,-?,--help              display this help message "
		"and exit";
	std::cout << std::endl;
	std::cout << "  -i,--instrument=<i>       use instrument string <i>";
	std::cout << std::endl;
	std::cout << "  -l,--longitude=<l>        longitude of the telescope "
		"[degrees]";
	std::cout << std::endl;
	std::cout << "  -L,--latitude=<l>         latitude of the telescope "
		"[degrees]";
	std::cout << std::endl;
	std::cout << "  -p,--post=<url>           post url to post the update";
	std::cout << std::endl;
	std::cout << "  -R,--rightascension=<r>   right ascension of the "
		"target [hours]";
	std::cout << std::endl;
	std::cout << "  -D,--declination=<d>      declination of the target "
		"[degrees]";
	std::cout << std::endl;
}

int	command_help(const char *progname) {
	usage(progname);
	return EXIT_SUCCESS;
}

int	command_monitor(GatewayPrx gateway) {
	// set up the monitor
	StatusUpdateMonitorI	*statusmonitor = new StatusUpdateMonitorI();
	statusmonitor->urlstring(urlstring);
	statusmonitor->execstring(execstring);

	// install the monitor in the ICE adapter
	Ice::ObjectPtr	monitor = statusmonitor;
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	CallbackAdapter	*_adapter = new CallbackAdapter(ic);
	Ice::Identity	ident = _adapter->add(monitor);
	gateway->ice_getConnection()->setAdapter(_adapter->adapter());

	// register the monitor
	gateway->registerMonitor(ident);

	signal(SIGINT, signal_handler);
	while (!completed) {
		sleep(1);
	}

	gateway->unregisterMonitor(ident);
	return EXIT_SUCCESS;
}

int	command_send(GatewayPrx gateway) {
	snowstar::StatusUpdate	update;
	update.updatetimeago = 3600 + 86400;
	update.avgguideerror = 1.1;
	update.currenttaskid = 4711;
	update.exposuretime = 12.91;
	update.filter = 3;
	update.telescope = convert(telescope);
	update.observatory = convert(location);
	update.instrument = instrument;
	gateway->send(update);
	return EXIT_SUCCESS;
}


static struct option	longopts[] = {
{ "debug",		no_argument,		NULL,		'd' },
{ "help",		no_argument,		NULL,		'h' },
{ "post",		required_argument,	NULL,		'p' },
{ "exec",		required_argument,	NULL,		'e' },
{ "foreground",		no_argument,		NULL,		'f' },
{ "instrument",		required_argument,	NULL,		'i' },
{ "rightascension",	required_argument,	NULL,		'R' },
{ "declination",	required_argument,	NULL,		'D' },
{ "longitude",		required_argument,	NULL,		'l' },
{ "latitude",		required_argument,	NULL,		'L' },
{ NULL,			0,			NULL,		 0  }
};

int	main(int argc, char *argv[]) {
	debug_set_ident("snowgateway");
	CommunicatorSingleton   cs(argc, argv);
	Ice::CommunicatorPtr    ic = CommunicatorSingleton::get();
	bool	foreground = false;
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "de:fh?i:p:R:D:l:L:",
		longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'e':
			execstring = std::string(optarg);
			break;
		case 'f':
			foreground = true;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
			break;
		case 'i':
			instrument = std::string(optarg);
			break;
		case 'p':
			urlstring = std::string(optarg);
			break;
		case 'D':
			telescope.dec() = astro::Angle(std::stod(optarg),
				astro::Angle::Degrees);
			break;
		case 'R':
			telescope.ra() = astro::Angle(std::stod(optarg),
				astro::Angle::Hours);
			break;
		case 'l':
			location.longitude() = astro::Angle(std::stod(optarg),
				astro::Angle::Degrees);
			break;
		case 'L':
			location.latitude() = astro::Angle(std::stod(optarg),
				astro::Angle::Degrees);
			break;
		}

	// next argument is help or servername
	if (argc <= optind) {
		throw std::runtime_error("server or command name missing");
	}
	std::string	command(argv[optind++]);
	if (command == "help") {
		command_help(argv[0]);
	}

	astro::ServerName	servername = astro::ServerName(command);
	if (argc <= optind) {
		throw std::runtime_error("command missing");
	}
	command = std::string(argv[optind++]);
	if (command == "help") {
		return command_help(argv[0]);
	}

	// go into the background if necessary
	if ((command == "monitor") && (!foreground)) {
		pid_t	pid = fork();
		if (pid < 0) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot fork(): %s",
				strerror(errno));
			return EXIT_FAILURE;
		}
		if (pid == 0) {
			return EXIT_SUCCESS;
		}
		if (pid > 0) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "backgrounded");
		}
	}

	// get the gateway interface
	Ice::ObjectPrx	base = ic->stringToProxy(servername.connect("Gateway"));
        GatewayPrx	gateway = GatewayPrx::checkedCast(base);

	if (command == "monitor") {
		return command_monitor(gateway);
	}

	if (command == "send") {
		return command_send(gateway);
	}

	return EXIT_SUCCESS;
}


} // namespace gateway
} // namespace app
} // namespace snowstar

int	main(int argc, char *argv[]) {
	int	rc = astro::main_function<snowstar::app::gateway::main>(argc,
			argv);
	snowstar::CommunicatorSingleton::release();
	return rc;
}
