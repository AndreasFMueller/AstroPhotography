/*
 * timeformat.cpp -- auxiliary function to format timestamps
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <includes.h>

namespace astro {

std::string	timeformat(const std::string& format, time_t when, bool local) {
	struct tm	*tmp = NULL;
	if (local) {
		tmp = localtime(&when);
	} else {
		tmp = gmtime(&when);
	}
	char	buffer[1024];
	strftime(buffer, sizeof(buffer), format.c_str(), tmp);
	return std::string(buffer);
}

} // namespace astro
