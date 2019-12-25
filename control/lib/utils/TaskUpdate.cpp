/*
 * TaskUpdate.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroGateway.h>
#include <AstroCamera.h>
#include <sstream>

namespace astro {
namespace gateway {

TaskUpdate::TaskUpdate(const std::string& instrument)
	: _instrument(instrument) {
	time(&updatetime);
	avgguideerror = 0;
	ccdtemperature = 0;
	lastimagestart = 0;
	exposuretime = -1;
	currenttaskid = 0;
	west = true;
	filter = -1;
	focus = -1;
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
	project = other.project;
	focus = other.focus;
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
	project = other.project;
	focus = other.focus;
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

	out << stringprintf("ccd temperature=%.1fºC", ccdtemperature - Temperature::zero);
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

	out << stringprintf("focus=%ld", focus);
	out << separator;

	out << "telescope=" << telescope.ra().hours();
	out << " " << telescope.dec().degrees();
	out << separator;

	out << "observatory=" << observatory.longitude().degrees();
	out << " " << observatory.latitude().degrees();
	out << separator;

	out << "project=" << project;

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
		stringprintf("%.1f", ccdtemperature - Temperature::zero)));
	tmp = localtime(&lastimagestart);
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
	result.insert(std::make_pair(std::string("focus"), 
		stringprintf("%ld", focus)));
	result.insert(std::make_pair(std::string("observatoryLONG"), 
		stringprintf("%.5f", observatory.longitude().degrees())));
	result.insert(std::make_pair(std::string("observatoryLAT"), 
		stringprintf("%.5f", observatory.latitude().degrees())));
	result.insert(std::make_pair(std::string("project"), project));
	return result;
}

} // namespace gateway
} // namespace astro
