/*
 * focusercommand.h -- focuser command implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <focusercommand.h>
#include <Ccds.h>
#include <iostream>
#include <unistd.h>

namespace astro {
namespace cli {

std::ostream&	operator<<(std::ostream& out, FocuserWrapper& focuser) {
	std::cout << "name:       " << focuser->getName() << std::endl;
	std::cout << "minimum:    " << focuser->min() << std::endl;
	std::cout << "current:    " << focuser->current() << std::endl;
	std::cout << "maximum:    " << focuser->max() << std::endl;
	return out;
}

void	focusercommand::info(FocuserWrapper& focuser,
		const std::vector<std::string>& arguments) {
	std::cout << focuser;
}

void	focusercommand::release(const std::string& focuserid,
		const std::vector<std::string>& arguments) {
	Focusers	focusers;
	focusers.release(focuserid);
}

void	focusercommand::assign(const std::string& focuserid,
		const std::vector<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "assign %s", focuserid.c_str());
	try {
		Focusers	focusers;
		focusers.assign(focuserid, arguments);
	} catch (std::exception& x) {
		throw command_error(x.what());
	}
}

void	focusercommand::set(FocuserWrapper& focuser,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 3) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"focuser command needs 3 arguments");
		throw command_error("position argument missing");
	}

	unsigned short	target = stoi(arguments[2]);
	focuser->set(target);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set focuser to %hu", target);

	if ((arguments.size() > 3) && (arguments[3] == "wait")) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "wait for completion of move");
		unsigned short	current = focuser->current();
		while (target != current) {
			usleep(1000);
			current = focuser->current();
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set command complete");
}

void	focusercommand::operator()(const std::string& commandname,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 2) {
		throw command_error("focuser command requires 2 arguments");
	}
	std::string	focuserid = arguments[0];
	std::string	subcommandname = arguments[1];
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"focuser command for focuser %s, subommand %s",
		focuserid.c_str(), subcommandname.c_str());
	if (subcommandname == "release") {
		release(focuserid, arguments);
		return;
	}
	if (subcommandname == "assign") {
		assign(focuserid, arguments);
		return;
	}

	// commands that need a focuser
	Focusers	focusers;
	FocuserWrapper	focuser = focusers.byname(focuserid);
	if (subcommandname == "info") {
		info(focuser, arguments);
		return;
	}
	if (subcommandname == "set") {
		set(focuser, arguments);
		return;
	}
	throw command_error("unknown command");
}

std::string	focusercommand::summary() const {
	return std::string("access focusers");
}

std::string	focusercommand::help() const {
	return std::string(
	"SYNOPSIS\n"
	"\n"
	"\tfocuser <focuserid> assign <name>\n"
	"\tfocuser <focuserid> info\n"
	"\tfocuser <focuserid> release\n"
	"\tfocuser <focuserid> set <position> [ wait ]\n"
	"\n"
	"DESCRIPTION\n"
	"\n"
	"The focuser command allows to get information about a focuser\n"
	"and set the current focuser position.\n"
	"The fourth synopsis sets a new position and optionally waits for\n"
	"the focuser position to be reached, if the wait keyword is given.\n"
	);
}

} // namespace cli
} // namespace astro
