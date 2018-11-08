/*
 * TaskUpdate.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTask.h>
#include <sstream>

namespace astro {
namespace task {

TaskUpdate::TaskUpdate(const std::string& instrument)
	: _instrument(instrument) {
	time(&updatetime);
}

TaskUpdate::TaskUpdate(const TaskUpdate& other)
	: _instrument(other._instrument) {
	updatetime = other.updatetime;
	avgguideerror = other.avgguideerror;
	ccdtemperature = other.ccdtemperature;
	lastimagestart = other.lastimagestart;
	exposuretime = other.exposuretime;
	currenttaskid = other.currenttaskid;
	telescope = other.telescope;
	west = other.west;
	filter = other.filter;
	observatory = other.observatory;
}

TaskUpdate&	TaskUpdate::operator=(const TaskUpdate& other) {
	_instrument = other._instrument;
	updatetime = other.updatetime;
	avgguideerror = other.avgguideerror;
	ccdtemperature = other.ccdtemperature;
	lastimagestart = other.lastimagestart;
	exposuretime = other.exposuretime;
	currenttaskid = other.currenttaskid;
	telescope = other.telescope;
	west = other.west;
	filter = other.filter;
	observatory = other.observatory;
	return *this;
}

/**
 * \brief convert the task update to a string
 *
 * \param separator to use between the items
 */
std::string	TaskUpdate::toString(std::string separator) const {
	std::ostringstream	out;

	out << "instrument=" << _instrument;
	out << separator;

	struct tm	*tmp = localtime(&updatetime);
	char	buffer[128];
	strftime(buffer, sizeof(buffer), "%F %T", tmp);
	out << "time=" << buffer;
	out << separator;

	out << stringprintf("guide error=%.1farcsec", avgguideerror);
	out << separator;

	out << stringprintf("ccd temperature=%.1fºC", ccdtemperature - 273.15);
	out << separator;

	tmp = localtime(&lastimagestart);
	strftime(buffer, sizeof(buffer), "%T", tmp);
	out << "last image start=" << buffer;
	out << separator;

	out << stringprintf("exposure time=%.3f", exposuretime);
	out << separator;

	out << stringprintf("current task=%d", currenttaskid);
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

TaskUpdate::operator	PostData() const {
	PostData	result;
	result.insert(std::make_pair(std::string("instrument"), _instrument));
	char	buffer[20];
	struct tm	*tmp = localtime(&updatetime);
	strftime(buffer, sizeof(buffer), "%F %T", tmp);
	result.insert(std::make_pair(std::string("updatetime"), 
		std::string(buffer)));
	result.insert(std::make_pair(std::string("avgguideerror"), 
		stringprintf("%.3f", avgguideerror)));
	result.insert(std::make_pair(std::string("ccdtemperature"), 
		stringprintf("%.1f", ccdtemperature - 273.15)));
	tmp = localtime(&updatetime);
	strftime(buffer, sizeof(buffer), "%F %T", tmp);
	result.insert(std::make_pair(std::string("lastimagestart"), 
		std::string(buffer)));
	result.insert(std::make_pair(std::string("exposuretime"), 
		stringprintf("%.3f", exposuretime)));
	result.insert(std::make_pair(std::string("currenttaskid"), 
		stringprintf("%d", currenttaskid)));
	result.insert(std::make_pair(std::string("telescopeRA"), 
		stringprintf("%.5f", telescope.ra().hours())));
	result.insert(std::make_pair(std::string("telescopeDEC"), 
		stringprintf("%.5f", telescope.dec().degrees())));
	result.insert(std::make_pair(std::string("west"), 
		(west) ? std::string("yes") : std::string("no")));
	result.insert(std::make_pair(std::string("filter"), 
		stringprintf("%d", filter)));
	result.insert(std::make_pair(std::string("observatoryLONG"), 
		stringprintf("%.5f", observatory.longitude().degrees())));
	result.insert(std::make_pair(std::string("observatoryLAT"), 
		stringprintf("%.5f", observatory.latitude().degrees())));
	return result;
}

} // namespace task
} // namespace astro
