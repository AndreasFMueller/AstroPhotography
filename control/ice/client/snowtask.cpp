/*
 * snowtask.cpp -- submit a task or monitor the execution of tasks on a server
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <CommunicatorSingleton.h>
#include <includes.h>
#include <AstroConfig.h>
#include <tasks.h>
#include <IceConversions.h>
#include <CommonClientTasks.h>

namespace snowstar {
namespace app {
namespace snowtask {

astro::ServerName	servername;
bool	completed = false;

void	signal_handler(int /* sig */) {
	completed = true;
}

static std::string	state2string(TaskState state) {
	switch (state) {
	case TskPENDING:
		return std::string("pending");
		break;
	case TskEXECUTING:
		return std::string("executing");
		break;
	case TskFAILED:
		return std::string("failed");
		break;
	case TskCANCELLED:
		return std::string("cancelled");
		break;
	case TskCOMPLETED:
		return std::string("completed");
		break;
	}
}

class TaskMonitorI : public TaskMonitor {
public:
	TaskMonitorI() {
		std::cout << "Date       Time     Id     new state";
		std::cout << std::endl;
	}
	void	stop(const Ice::Current& /* current */) {	
		kill(getpid(), SIGINT);
	}
	void	update(const TaskMonitorInfo& info,
		const Ice::Current& /* current */) {
		time_t	t = converttime(info.timeago);
		std::cout << astro::timeformat("%Y-%m-%d %H:%M:%S", t);
		std::cout << astro::stringprintf(" %6d %s", info.taskid,
			state2string(info.newstate).c_str());
		std::cout << std::endl;
	}
};



/**
 * \brief Implementation of the monitor command
 */
int	command_monitor() {

	// get the Tasks interface
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
        Ice::ObjectPrx  base = ic->stringToProxy(servername.connect("Tasks"));
	TaskQueuePrx	tasks = TaskQueuePrx::checkedCast(base);

	// create a monitor callback
	Ice::ObjectPtr	callback = new TaskMonitorI();

	// register the callback with the server
	CallbackAdapter	adapter(ic);
	Ice::Identity	ident = adapter.add(callback);
	tasks->ice_getConnection()->setAdapter(adapter.adapter());
	tasks->registerMonitor(ident);

	// wait for the termination
	signal(SIGINT, signal_handler);
	while (!completed) {
		sleep(1);
	}

	// unregister callback before exiting
	tasks->unregisterMonitor(ident);

	// that's it
	return EXIT_SUCCESS;
}

/**
 * \brief Usage function for the snowtask program
 */
void	usage(const char *progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "usage:" << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -d,--debug     increase debug level" << std::endl;
	std::cout << " -h,--help      show this help and exit" << std::endl;
}

/**
 * \brief Options for the snowtask program
 */
static struct option	longopts[] = {
{ "config",	required_argument,	NULL,		'c' }, /* 0 */
{ "debug",	no_argument,		NULL,		'd' }, /* 1 */
{ "help",	no_argument,		NULL,		'h' }, /* 2 */
{ "server",	required_argument,	NULL,		's' }, /* 3 */
{ NULL,		0,			NULL,		0   }
};

/**
 * \brief Main function for the snowtask program
 */
int	main(int argc, char *argv[]) {
	CommunicatorSingleton	cs(argc, argv);
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();

	astro::ServerName	servername;

	// parse command line options
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "c:dh?s:", longopts,
		&longindex)))
		switch (c) {
		case 'c':
			astro::config::Configuration::set_default(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 's':
			servername = astro::ServerName(optarg);
			break;
		}

	// get the command name
	if (argc <= optind) {
		throw std::runtime_error("command name missing");
	}
	std::string	command = argv[optind++];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "working on command %s",
		command.c_str());

	// the monitor command does not need any other 
	if (command == "monitor") {
		return command_monitor();
	}

	return EXIT_SUCCESS;
}

} // namespace snowtask
} // namespace app
} // namespace snowtar

int	main(int argc, char *argv[]) {
	return astro::main_function<snowstar::app::snowtask::main>(argc, argv);
}
