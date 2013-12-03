/*
 * taskcommand.cpp -- task command
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <guidecli.h>
#include <taskcommand.h>
#include <AstroDebug.h>
#include <CorbaExceptionReporter.h>
#include <tasks.hh>
#include <Output.h>

namespace astro {
namespace cli {

void	taskcommand::operator()(const std::string& command,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 2) {
		throw command_error("task command requires arguments");
	}
	std::string	taskidstring = arguments[0];
	int	taskid = stoi(taskidstring);
	std::string	subcommand = arguments[1];

	if (subcommand == "info") {
		info(taskid);
		return;
	}
}

static std::ostream&	operator<<(std::ostream& out, Astro::Exposure& exposure) {
	out << "exposure time:  " << exposure.exposuretime << std::endl;
	out << "gain:           " << exposure.gain << std::endl;
	out << "limit:          " << exposure.limit << std::endl;
	out << "shutter:        ";
	switch (exposure.shutter) {
	case Astro::SHUTTER_CLOSED:	out << "closed"; break;
	case Astro::SHUTTER_OPEN:	out << "open"; break;
	}
	out << std::endl;
	out << "binning:        " << exposure.mode << std::endl;
	out << "origin:         " << exposure.frame.origin << std::endl;
	out << "size:           " << exposure.frame.size << std::endl;
	return out;
}

static std::ostream&	operator<<(std::ostream& out, Astro::TaskParameters_var parameters) {
	out << "camera:         " << parameters->camera << std::endl;
	out << "ccd:            " << parameters->ccdid << std::endl;
	out << "temperature:    " << parameters->ccdtemperature << std::endl;
	out << "filterwheel:    " << parameters->filterwheel << std::endl;
	out << "filterposition: " << parameters->filterposition << std::endl;
	out << parameters->exp;
	return out;
}

void	taskcommand::info(int taskid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "info about task %d", taskid);
	guidesharedcli	gcli;
	try {
		Astro::Task_var	task = gcli->taskqueue->getTask(taskid);
		Astro::TaskParameters_var	parameters
			= task->parameters();
		std::cout << parameters;
		std::cout << "image name:     " << task->imagename() << std::endl;
	} catch (...) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found task");
	}
}

std::string	taskcommand::summary() const {
	return std::string("manipulate tasks");
}

std::string	taskcommand::help() const {
	return std::string(
		"SYNOPSIS\n"
		"\n"
		"\ttask <id> info\n"
		"\n"
		"DESCRIPTION\n"
		"\n"
		"Display information about a task\n"
	);
}

} // namespace cli
} // namespace astro
