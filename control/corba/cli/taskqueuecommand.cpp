/*
 * taskqueuecommand.cpp -- o
 * 
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <taskqueuecommand.h>
#include <guidecli.h>
#include <AstroDebug.h>
#include <unistd.h>

namespace astro {
namespace cli {

void	taskqueuecommand::start(Astro::TaskQueue_var& taskqueue) {
	taskqueue->start();
}

void	taskqueuecommand::stop(Astro::TaskQueue_var& taskqueue) {
	taskqueue->stop();
}

void	taskqueuecommand::state(Astro::TaskQueue_var& taskqueue) {
	Astro::TaskQueue::QueueState	state = taskqueue->state();
	switch (state) {
	case Astro::TaskQueue::IDLE:
		std::cout << "idle" << std::endl;
		break;
	case Astro::TaskQueue::LAUNCHING:
		std::cout << "launching" << std::endl;
		break;
	case Astro::TaskQueue::STOPPING:
		std::cout << "stopping" << std::endl;
		break;
	case Astro::TaskQueue::STOPPED:
		std::cout << "stopped" << std::endl;
		break;
	}
}

void	taskqueuecommand::wait(Astro::TaskQueue_var& taskqueue) {
	try {
		Astro::TaskQueue::QueueState	state = taskqueue->state();
		if (Astro::TaskQueue::STOPPED == state) {
			std::cout << "queue already stopped" << std::endl;
			return;
		}
		if (Astro::TaskQueue::STOPPING != state) {
			std::cout << "queue is not stopping, cannot wait"
				<< std::endl;
			return;
		}
		while (Astro::TaskQueue::STOPPED != taskqueue->state()) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"waiting for queue to stop");
			usleep(100000);
		}
		std::cout << "queue stopped" << std::endl;
	} catch (...) {
		std::cerr << "error while waiting for queue to stop";
	}
}

void	taskqueuecommand::operator()(const std::string& /* command */,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 1) {
		throw std::runtime_error("taskqueue command needs argument");
	}
	std::string	subcommand = arguments[0];
	guidesharedcli	gcli;
	if ("start" == subcommand) {
		start(gcli->taskqueue);
		return;
	}
	if ("stop" == subcommand) {
		stop(gcli->taskqueue);
		return;
	}
	if ("state" == subcommand) {
		state(gcli->taskqueue);
		return;
	}
	if ("wait" == subcommand) {
		wait(gcli->taskqueue);
		return;
	}
}

std::string	taskqueuecommand::summary() const {
	return std::string("task queue management");
}

std::string	taskqueuecommand::help() const {
	return std::string(
		"SYNOPSIS\n"
		"\n"
		"\ttaskqueue start\n"
		"\ttaskqueue stop\n"
		"\ttaskqueue state\n"
		"\ttaskqueue wait\n"
		"\n"
		"DESCRIPTION\n"
		"\n"
		"Start, stop or query the state of the queue.\n"
	);
}

} // namespace cli
} // namespace astro


