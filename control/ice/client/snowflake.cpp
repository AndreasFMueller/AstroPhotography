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

using namespace snowstar;

namespace snowflake {

int	main(int argc, char *argv[]) {
	int	status = EXIT_FAILURE;
	Ice::CommunicatorPtr	ic;
	try {
		ic = Ice::initialize(argc, argv);
	} catch (...) {
		throw;
	}

	// parse command line
	int	c;
	std::string	modulename;
	while (EOF != (c = getopt(argc, argv, "dm:")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		}

	try {
		Ice::ObjectPrx	base
			= ic->stringToProxy("Tasks:default -h othello -p 10000");
		snowstar::TaskQueuePrx	tasks = TaskQueuePrx::checkedCast(base);
		if (!tasks) {
			throw "invalid proxy";
		}
		taskidsequence sequence = tasks->tasklist(TskCOMPLETED);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "number of tasks: %d",
			sequence.size());
		taskidsequence::const_iterator	i;
		for (i = sequence.begin(); i != sequence.end(); i++) {
			TaskInfo	info = tasks->info(*i);
			std::cout << "id:     " << info.taskid << std::endl;
			std::cout << "last:   " << info.lastchange << std::endl;
			std::cout << "cause:  " << info.cause << std::endl;
			std::cout << "file:   " << info.filename << std::endl;

			TaskParameters	parm = tasks->parameters(*i);
			std::cout << "camera: " << parm.camera << std::endl;
			std::cout << "ccd:    " << parm.ccdid << std::endl;
			std::cout << "temp:   " << parm.ccdtemperature << std::endl;
			std::cout << "fw:     " << parm.filterwheel << std::endl;
			std::cout << "filter: " << parm.filterposition << std::endl;

			TaskPrx	task = tasks->getTask(*i);
			std::cout << "file2:  " << task->imagename() << std::endl;
			std::cout << std::endl;
			
		}
		
		status = EXIT_SUCCESS;
	} catch (const Ice::Exception& icex) {
		std::cerr << icex << std::endl;
	} catch (const char *msg) {
		std::cerr << msg << std::endl;
	}
	if (ic) {
		ic->destroy();
	}
	return status;
}

} // namespace snowflake

int	main(int argc, char *argv[]) {
	try {
		return snowflake::main(argc, argv);
	} catch (...) {
		std::cerr << "unknown exception" << std::endl;
	}
}