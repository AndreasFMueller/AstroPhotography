/*
 * sleepcommand.h -- sleep command implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <sleepcommand.h>
#include <unistd.h>

namespace astro {
namespace cli {

void	sleepcommand::operator()(const std::string& commandname,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 1) {
		throw command_error("sleep command requires time argument");
	}

	std::string	timestring = arguments[0];
	useconds_t	sleeptime = 1000000 * stof(timestring);
	usleep(sleeptime);
}

std::string	sleepcommand::summary() const {
	return std::string("pause execution for some time");
}

std::string	sleepcommand::help() const {
	return std::string(
	"SYNOPSIS\n"
	"\n"
	"\tsleep time\n"
	"\n"
	"DESCRIPTION\n"
	"\n"
	"The sleep command pauses the execution of the script for some time.\n"
	"The time is specified in seconds, as a floating point number, and\n"
	"has the same time resolution as the usleep(2) function of the host\n"
	"operating system.\n"
	);
}

} // namespace cli
} // namespace astro
