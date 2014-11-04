/*
 * helpcommand.cpp -- help command
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <guidecli.h>
#include <helpcommand.h>
#include <AstroDebug.h>

namespace astro {
namespace cli {

void	helpcommand::operator()(const std::string& /* command */,
		const std::vector<std::string>& arguments) {
	if (arguments.size() == 0) {
		std::cout << factory().summary() << std::endl;
		return;
	}
	std::string	commandname = arguments[0];
	std::vector<std::string>	helparguments(arguments);
	helparguments.erase(helparguments.begin());
	std::cout << factory().help(commandname, helparguments);
}

std::string	helpcommand::summary() const {
	return std::string("general help or help about commands");
}

std::string	helpcommand::help() const {
	return std::string(
		"SYNOPSIS\n"
		"\n"
		"\thelp [ command ]\n"
		"\n"
		"DESCRIPTION\n"
		"\n"
		"display generic help (without argument) or help for a\n"
		"specific command given as the argument.\n"
	);
}

} // namespace cli
} // namespace astro
