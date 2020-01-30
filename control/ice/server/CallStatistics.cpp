/*
 * CallStatistics.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil 
 */
#include "StatisticsI.h"
#include <list>
#include <AstroFormat.h>

namespace snowstar {

std::map<std::string, CallStatisticsPtr>	CallStatistics::call_statistics;

/**
 * \brief Retrieve the number of calls to a particular call of a servant
 *
 * \param servantname	name of the servant
 * \param callname	name of the call
 */
long	CallStatistics::calls(const std::string& servantname,
		const std::string& callname) {
	auto	i = call_statistics.find(servantname);
	if (i == call_statistics.end()) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf("servant '%s' not found",
			servantname.c_str());
		throw notfound;
	}
	auto	j = i->second->find(callname);
	if (j == i->second->end()) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf("call '%s.%s' not found",
			servantname.c_str(), callname.c_str());
		throw notfound;
	}
	return j->second;
}

/**
 * \brief Count the number of calls to a particular call of a servant
 *
 * \param servantname	name of the servant
 * \param callname	name of the call
 */
void	CallStatistics::count(const std::string& servantname,
		const std::string& callname) {
	auto	i = call_statistics.find(servantname);
	if (i == call_statistics.end()) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf("servant '%s' not found",
			servantname.c_str());
		throw notfound;
	}
	auto	j = i->second->find(callname);
	if (j == i->second->end()) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf("call '%s.%s' not found",
			servantname.c_str(), callname.c_str());
		throw notfound;
	}
	j->second++;
}

/**
 * \brief Construct a list of servant names
 */
std::list<std::string>	CallStatistics::servantnames() {
	std::set<std::string>	sn;
	std::for_each(call_statistics.begin(), call_statistics.end(),
		[&sn](const std::pair<std::string, CallStatisticsPtr>& p) mutable {
			sn.insert(p.first);
		}
	);
	std::list<std::string>	sn2;
	std::copy(sn.begin(), sn.end(), std::back_inserter(sn2));
	return sn2;
}
 
/**
 * \brief Construct a list of call names for a particular servant
 *
 * \param servantname	name of the servant
 */
std::list<std::string>	CallStatistics::callnames(const std::string& servantname) {
	auto	i = call_statistics.find(servantname);
	if (i == call_statistics.end()) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf("servant '%s' not found",
			servantname.c_str());
		throw notfound;
	}
	std::set<std::string>	cn;
	std::for_each(i->second->begin(), i->second->end(),
		[&cn](const std::pair<std::string, long>& j) mutable {
			cn.insert(j.first);
		}
	);
	std::list<std::string>	cn2;
	std::copy(cn.begin(), cn.end(), std::back_inserter(cn2));
	return cn2;
}

void	CallStatistics::remember(const std::string& servantname,
		CallStatisticsPtr callstatistics) {
	call_statistics.insert(std::make_pair(servantname, callstatistics));
}

CallStatisticsPtr	CallStatistics::recall(const std::string& servantname) {
	auto	i = call_statistics.find(servantname);
	if (i == call_statistics.end()) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf("servant '%s' not found",
			servantname.c_str());
		throw notfound;
	}
	return i->second;
}

} // namespace snowstar
