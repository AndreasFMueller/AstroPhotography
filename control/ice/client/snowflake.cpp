/*
 * snowflake.cpp -- a test client for the snowstar server
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Ice/Ice.h>
#include <device.h>
#include <cstdlib>
#include <iostream>
#include <AstroDebug.h>
#include <tasks.h>
#include <AstroUtils.h>
#include <CommonClientTasks.h>
#include <CommunicatorSingleton.h>
#include <IceConversions.h>
#include <includes.h>
#include <time.h>

using namespace snowstar;

namespace snowstar {
namespace app {
namespace snowflake {

static struct option	longopts[] = {
{ "debug",		no_argument,		NULL,	'd' }, /* 0 */
{ "help",		no_argument,		NULL,	'h' }, /* 1 */
};

/**
 * \brief Display a help message
 */
static void	usage(const char *progname) {
	astro::Path	path(progname);
	std::cout << "usage: " << path.basename() << " [ options ] server "
		<< std::endl;
	std::cout << "retrieve a list of tasks from the servier <server>"
		<< std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -d,--debug          enable debug output" << std::endl;
}

int	main(int argc, char *argv[]) {
	debug_set_ident("snowflake");
	snowstar::CommunicatorSingleton	cs(argc, argv);
	Ice::CommunicatorPtr	ic = snowstar::CommunicatorSingleton::get();

	// parse command line
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "dh",
		longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		default:
			throw std::runtime_error("unknown option");
		}

	// next argument is mandatory
	if (optind >= argc) {
		std::cerr << "missing service name argument" << std::endl;
		return EXIT_FAILURE;
	}
	astro::ServerName	servername(argv[optind++]);

	// now contact the service
	Ice::ObjectPrx	base
		= ic->stringToProxy(servername.connect("Tasks"));
	snowstar::TaskQueuePrx	tasks = TaskQueuePrx::checkedCast(base);
	if (!tasks) {
		throw "invalid proxy";
	}
	taskidsequence sequence = tasks->tasklist(TskCOMPLETE);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "number of tasks: %d",
		sequence.size());
	taskidsequence::const_iterator	i;
	for (i = sequence.begin(); i != sequence.end(); i++) {
		TaskInfo	info = tasks->info(*i);
		std::cout << "id:     " << info.taskid << std::endl;
		time_t	when = converttime(info.lastchange);
		std::cout << "last:   " << ctime(&when); // << std::endl;
		std::cout << "cause:  " << info.cause << std::endl;
		std::cout << "file:   " << info.filename << std::endl;

		TaskParameters	parm = tasks->parameters(*i);
		std::cout << "camera: " << info.camera << std::endl;
		std::cout << "ccd:    " << info.ccd << std::endl;
		std::cout << "cooler: " << info.cooler << std::endl;
		std::cout << "temp:   " << parm.ccdtemperature << std::endl;
		std::cout << "fw:     " << info.filterwheel << std::endl;
		std::cout << "filter: " << parm.filter << std::endl;

		TaskPrx	task = tasks->getTask(*i);
		std::cout << "file2:  " << task->imagename() << std::endl;
		std::cout << std::endl;
	}
	return EXIT_SUCCESS;
}

} // namespace snowflake
} // namespace app
} // namespace snowstar

int	main(int argc, char *argv[]) {
	int	rc = astro::main_function<snowstar::app::snowflake::main>(argc,
			argv);
	CommunicatorSingleton::release();
	return rc;
}
