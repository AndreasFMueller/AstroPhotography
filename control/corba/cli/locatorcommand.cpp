/*
 * locatorcommand.cpp -- commands related to the locator
 *
 * (c) 2013 Prof Dr Andreas Muelelr, Hochschule Rapperswil
 */
#include <locatorcommand.h>

namespace astro {
namespace cli {

void	locatorcommand::operator()(const std::string& /* command */,
		const std::vector<std::string>& /* arguments */) {
}

std::string	locatorcommand::summary() const {
	return std::string("locate devices");
}

std::string	locatorcommand::help() const {
	return std::string(
		"SYNOPSIS\n"
		"\n"
		"\tlocate\n"
		"\n"
		"DESCRIPTION\n"
		"\n"
		"Locate a module and devices associated with it\n"
	);
}

} // namespace cli
} // namespace astro
