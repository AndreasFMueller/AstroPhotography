/*
 * TaskUpdate.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTask.h>
#include <sstream>

namespace astro {
namespace task {

/**
 * \brief convert the task update to a string
 *
 * \param separator to use between the items
 */
std::string	TaskUpdate::toString(std::string separator) const {
	std::ostringstream	out;

	struct tm	*tmp = localtime(&updatetime);
	char	buffer[128];
	strftime(buffer, sizeof(buffer), "%F %T", tmp);
	out << "time=" << buffer;
	out << separator;

	out << stringprintf("guide error=%.1farcsec", avgguideerror);
	out << separator;

	out << stringprintf("ccd temperatur=%.1fºC", ccdtemperature - 273.15);
	out << separator;

	tmp = localtime(&lastimagestart);
	strftime(buffer, sizeof(buffer), "%T", tmp);
	out << "last image start=" << buffer;
	out << separator;

	out << stringprintf("exposure time=%.3f", exposuretime);
	out << separator;

	out << stringprintf("current task=%d", currenttaskid);
	out << separator;

	out << stringprintf("pending tasks=%d", pendingtasks);
	out << separator;

	out << stringprintf("filter=%d", filter);
	out << separator;

	out << "telescope=" << telescope.ra().hours();
	out << " " << telescope.dec().degrees();
	out << separator;

	out << "observatory=" << observatory.longitude().degrees();
	out << " " << observatory.latitude().degrees();

	return out.str();
}

} // namespace task
} // namespace astro
