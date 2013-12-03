/*
 * tasktest.cpp -- test the task queue process
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTask.h>
#include <AstroDebug.h>
#include <cstdlib>
#include <unistd.h>

using namespace astro::persistence;

namespace astro {
namespace task {

/**
 * \brief main function for the task test program
 */
int	main(int argc, char *argv[]) {
	debugthreads = 1;
	// parse the command line arguments
	int	c;
	while (EOF != (c = getopt(argc, argv, "d")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		}

	// create the database
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating the database");
	DatabaseFactory	factory;
	Database	database = factory.get("testdb.db");

	// create the task queue
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create taskqueue");
	TaskQueue	queue(database);

	// submit a job to the task queue
	Task	task;
	task.camera("simulator:camera");
	task.filterwheel("simulator:filterwheel");
	task.filterposition(0);
	task.ccdtemperature(260);
	camera::Exposure	exposure = task.exposure();

	// wait for some time
	int	taskcounter = 4;
	while (--taskcounter) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"new task with exposure time %d", taskcounter);
		exposure.exposuretime = taskcounter;
		task.exposure(exposure);
		int queueid = queue.submit(task);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "id %d submitted", queueid);
	}
	sleep(10);

	// stop the queue
	queue.stop();
	queue.wait();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "wait complete");
	sleep(1);

	// submit another task
	exposure.exposuretime = 2;
	task.exposure(exposure);
	queue.start();
	queue.submit(task);
	sleep(1);

	// immediately cancel everything
	queue.stop();
	queue.cancel();
	queue.wait();
	queue.shutdown();

	// that's it
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end test");
	return EXIT_SUCCESS;
}

} // namespace task
} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::task::main(argc, argv);
	} catch (std::exception& x) {
	}
	return EXIT_FAILURE;
}
