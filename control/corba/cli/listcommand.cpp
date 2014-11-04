/*
 * list.cpp -- list commands
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <guidecli.h>
#include <listcommand.h>
#include <AstroDebug.h>
#include <CorbaExceptionReporter.h>

namespace astro {
namespace cli {

void	listcommand::operator()(const std::string& /* command */,
		const std::vector<std::string>& arguments) {
	if (arguments.size() == 0) {
		throw command_error("list command requires arguments");
	}
	if (arguments[0] == std::string("modules")) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "list modules command");
		try {
			listmodules();
		} catch (CORBA::Exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"list modules throws exception: %s",
				Astro::exception2string(x).c_str());
		}
		return;
	}
	if (arguments[0] == std::string("images")) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "list images command");
		try {
			listimages();
		} catch (CORBA::Exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"list images throws exception: %s",
				Astro::exception2string(x).c_str());
		}
		return;
	}
	if (arguments[0] == std::string("tasks")) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "list tasks command");
		try {
			listtasks(arguments);
		} catch (CORBA::Exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"list tasks throws exception: %s",
				Astro::exception2string(x).c_str());
		}
		return;
	}
	throw command_error("cannot execute list command");
}

void	listcommand::listmodules() {
	// get the modules object
	guidesharedcli	gcli;
	Astro::Modules::ModuleNameSequence_var	namelist;
	try {
		namelist = gcli->modules->getModuleNames();
	} catch (const CORBA::Exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"getModuleNames throws exception: %s",
			Astro::exception2string(x).c_str());
		return;
	}
	for (int i = 0; i < (int)namelist->length(); i++) {
		std::cout << namelist[i] << std::endl;
	}
}

class imageinfo {
public:
	std::string	name;
	imageinfo(const char *n) : name(n) { }
	long	age;
	long	size;
};

std::ostream&	operator<<(std::ostream& out, const imageinfo& info) {
	char	buffer[81];
	time_t	now = time(NULL) - info.age;;
	struct tm	*t = localtime(&now);;
	int	offset = snprintf(buffer, sizeof(buffer),
			"%-40.40s  %10ld     ", info.name.c_str(), info.size);
	strftime(buffer + offset, sizeof(buffer) - offset,
		"%Y-%m-%d  %H:%M:%S", t);
	out << buffer;
	return out;
}

void	listcommand::listimages() {
	guidesharedcli	gcli;
	Astro::Images::ImageList_var	namelist;
	try {
		namelist = gcli->images->listImages();
	} catch (const CORBA::Exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"getModuleNames throws exception: %s",
			Astro::exception2string(x).c_str());
		return;
	}
	for (int i = 0; i < (int)namelist->length(); i++) {
		imageinfo	info(namelist[i]);
		info.size = gcli->images->imageSize(info.name.c_str());
		info.age = gcli->images->imageAge(info.name.c_str());
		std::cout << info << std::endl;
	}
}

void	listcommand::listtasks(const std::vector<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "list tasks");
	guidesharedcli	gcli;
	Astro::TaskState	state = Astro::TASK_COMPLETED;
	if (arguments.size() > 2) {
		std::string	statestring = arguments[2];
		if (statestring == "pending") {
			state = Astro::TASK_PENDING;
		}
		if (statestring == "executing") {
			state = Astro::TASK_EXECUTING;
		}
		if (statestring == "failed") {
			state = Astro::TASK_FAILED;
		}
		if (statestring == "cancelled") {
			state = Astro::TASK_CANCELLED;
		}
		if (statestring == "completed") {
			state = Astro::TASK_COMPLETED;
		}
	}
	Astro::TaskQueue::taskidsequence_var	taskids
		= gcli->taskqueue->tasklist(state);
	for (unsigned int i = 0; i < taskids->length(); i++) {
		std::cout << taskids[i] << std::endl;
	}
}

std::string	listcommand::summary() const {
	return std::string("list various object types");
}

std::string	listcommand::help() const {
	return std::string(
		"SYNOPSIS\n"
		"\n"
		"\tlist <type>\n"
		"\n"
		"DESCRIPTION\n"
		"\n"
		"Display a list of objects of a given <type>. Valid <type>\n"
		"values are \"modules\", \"images\" and \"tasks\".\n"
	);
}

} // namespace cli
} // namespace astro
