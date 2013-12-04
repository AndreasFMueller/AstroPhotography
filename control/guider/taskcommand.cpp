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
#include <Conversions.h>

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
	if (subcommand == "parameters") {
		parameters(taskid);
		return;
	}
}

static std::ostream&	operator<<(std::ostream& out,
				Astro::Exposure& exposure) {
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
	out << "frame:          " << exposure.frame << std::endl;
	return out;
}

static std::ostream&	operator<<(std::ostream& out,
		Astro::TaskParameters_var parameters) {
	out << "camera:         " << parameters->camera << std::endl;
	out << "ccd:            " << parameters->ccdid << std::endl;
	out << "temperature:    " << parameters->ccdtemperature << std::endl;
	out << "filterwheel:    " << parameters->filterwheel << std::endl;
	out << "filterposition: " << parameters->filterposition << std::endl;
	out << parameters->exp;
	return out;
}

static std::ostream&	operator<<(std::ostream& out, Astro::TaskInfo_var info) {
	out << "task id:        " << info->taskid << std::endl;
	out << "state:          " << astro::convert(info->state) << std::endl;
	out << "lastchange:     ";
        char    buffer[81];
        time_t  lastchange = time(NULL) - info->lastchange;;
        struct tm       *t = localtime(&lastchange);;
        strftime(buffer, sizeof(buffer), "%Y-%m-%d  %H:%M:%S", t);
        out << buffer << std::endl;
	out << "filename:       " << info->filename << std::endl;
	return out;
}

void	taskcommand::info(int taskid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "info about task %d", taskid);
	guidesharedcli	gcli;
	try {
		Astro::Task_var	task = gcli->taskqueue->getTask(taskid);
		Astro::TaskInfo_var	info = task->info();
		std::cout << info;
	} catch (...) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found task");
	}
}

void	taskcommand::parameters(int taskid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "info about task %d", taskid);
	guidesharedcli	gcli;
	try {
		Astro::Task_var	task = gcli->taskqueue->getTask(taskid);
		Astro::TaskParameters_var	parameters
			= task->parameters();
		std::cout << parameters;
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
		"\ttask <id> parameters\n"
		"\n"
		"DESCRIPTION\n"
		"\n"
		"Display information about a task\n"
	);
}

} // namespace cli
} // namespace astro
