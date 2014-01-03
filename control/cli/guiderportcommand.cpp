/*
 * guiderportcommand.cpp -- guiderport command implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <guiderportcommand.h>
#include <AstroDebug.h>
#include <map>
#include <camera.hh>
#include <OrbSingleton.h>
#include <iostream>
#include <ObjWrapper.h>
#include <DeviceMap.h>
#include <CorbaExceptionReporter.h>
#include <Guiderports.h>
#include <string>

namespace astro {
namespace cli {

//////////////////////////////////////////////////////////////////////
// guiderportcommand implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief release a guiderport 
 */
void	guiderportcommand::release(const std::string& guiderportid,
		const std::vector<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guiderport release subcommand");
	Guiderports	guiderports;
	guiderports.release(guiderportid);
}

/**
 * \brief activate guider port outputs
 */
void	guiderportcommand::activate(const std::string& guiderportid,
		const std::vector<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guiderport activate subcommand");
	Guiderports	guiderports;
	GuiderPortWrapper	guiderport = guiderports.byname(guiderportid);
	if (arguments.size() < 4) {
		std::cerr << "activate command requires 3 arguments";
		std::cerr << std::endl;
	}
	double	ra = stod(arguments[2]);
	double	dec = stod(arguments[3]);
	guiderport->activate(ra, dec);
}

/**
 * \brief Assign a guiderport to a name
 */
void	guiderportcommand::assign(const std::string& guiderportid,
		const std::vector<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guiderport assign subcommand");
	Guiderports	guiderports;
	guiderports.assign(guiderportid, arguments);
}

/**
 * \brief execute a subcommand
 */
void	guiderportcommand::operator()(const std::string& commandname,
		const std::vector<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guiderport command");

	if (arguments.size() < 2) {
		throw command_error("guiderport command requires 2 arguments");
	}

	std::string	guiderportid = arguments[0];
	std::string	subcommandname = arguments[1];

	if (subcommandname == "activate") {
		activate(guiderportid, arguments);
		return;
	}

	if (subcommandname == "release") {
		release(guiderportid, arguments);
		return;
	}

	if (subcommandname == "assign") {
		assign(guiderportid, arguments);
		return;
	}

	throw command_error("guiderport subcommand not known");

}

std::string	guiderportcommand::summary() const {
	return std::string("access guiderports");
}

std::string	guiderportcommand::help() const {
	return std::string(
	"SYNOPSIS\n"
	"\n"
	"\tguiderport <guiderportid> assign <guiderportname>\n"
	"\tguiderport <guiderportid> activate <ra+> <ra-> <dec+> <dec-> \n"
	"\tguiderport <guiderportid> release\n"
	"\n"
	"DESCRIPTION\n"
	"\n"
	"The guiderport command identifies guiderports using a guiderport id, which\n"
	"is always given as the first argument of the command. The user is\n"
	"free to choose the guiderport id, but it should consist only of alpha-\n"
	"numeric characters.\n"
	"\n"
	"The second synopsis activates the outputs of the guider port for\n"
	"the specified time in seconds\n"
	"\n"
	"The third synopsis tells the system that the guiderport with name\n"
	"<guiderportid> is no longer needed.\n"
	);
}

} // namespace cli
} // namespace astro
