/*
 * CallStatistics.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil 
 */
#include "StatisticsI.h"
#include <list>
#include <AstroFormat.h>
#include <Ice/Initialize.h>
#include <AstroDebug.h>

namespace snowstar {

std::map<Ice::Identity, CallStatisticsPtr>	CallStatistics::call_statistics;

/**
 * \brief Construct a list of known object identities
 */
std::list<Ice::Identity>	CallStatistics::objectidentities() {
	std::set<Ice::Identity>	sn;
	std::for_each(call_statistics.begin(), call_statistics.end(),
		[&sn](const std::pair<Ice::Identity, CallStatisticsPtr>& p) mutable {
			sn.insert(p.first);
		}
	);
	std::list<Ice::Identity>	sn2;
	std::copy(sn.begin(), sn.end(), std::back_inserter(sn2));
	return sn2;
}

/**
 * \brief Return the number of known object identities
 */
unsigned long	CallStatistics::objectidentityCount() {
	return call_statistics.size();
}

/**
 * \brief Construct a list of call names for a particular servant
 *
 * \param objectidentity	name of the servant
 */
std::list<std::string>	CallStatistics::operations(
				const Ice::Identity& objectidentity) {
	auto	i = call_statistics.find(objectidentity);
	if (i == call_statistics.end()) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf("servant '%s' not found",
			Ice::identityToString(objectidentity).c_str());
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

/**
 * \brief Return number of known operations on an object
 *
 * \param objectidentity	the object identity to query
 */
unsigned long	CallStatistics::operationCount(
			const Ice::Identity& objectidentity) {
	auto	i = call_statistics.find(objectidentity);
	if (i == call_statistics.end()) {
		return 0;
	}
	return i->second->size();
}

/**
 * \brief Return number of all calls on a given object identity
 *
 * \param objectidenity
 */
unsigned long	CallStatistics::calls(const Ice::Identity& objectidentity) {
	auto	i = call_statistics.find(objectidentity);
	if (i == call_statistics.end()) {
		return 0;
	}
	return i->second->calls();
}

/**
 * \brief Retrieve the number of calls to a particular call of a servant
 *
 * \param objectidentity	name of the servant
 * \param operation	name of the call
 */
unsigned long	CallStatistics::calls(const Ice::Identity& objectidentity,
		const std::string& operation) {
	auto	i = call_statistics.find(objectidentity);
	if (i == call_statistics.end()) {
		return 0;
	}
	auto	j = i->second->find(operation);
	if (j == i->second->end()) {
		return 0;
	}
	return j->second;
}

/**
 * \brief Count the number of calls to a particular call of a servant
 *
 * \param objectidentity	name of the servant
 * \param operation	name of the call
 */
void	CallStatistics::count(const Ice::Identity& objectidentity,
		const std::string& operation) {
	CallStatisticsPtr	s;
	auto	i = call_statistics.find(objectidentity);
	if (i == call_statistics.end()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "register new object id %s",
			Ice::identityToString(objectidentity).c_str());
		s = CallStatisticsPtr(new CallStatistics(objectidentity));
		call_statistics.insert(std::make_pair(objectidentity, s));
	} else {
		s = i->second;
	}
	s->count(operation);
}

/**
 * \brief Count based on context
 */
void	CallStatistics::count(const Ice::Current& current) {
	count(current.id, current.operation);
}

/**
 * \brief Get Call statistics object for an object
 *
 * This method creates an entry in the map if the object identity is not
 * already present.
 *
 * \param objectidentity	object identity to query
 */
CallStatisticsPtr	CallStatistics::recall(
				const Ice::Identity& objectidentity) {
	auto	i = call_statistics.find(objectidentity);
	if (i == call_statistics.end()) {
		CallStatisticsPtr	s(new CallStatistics(objectidentity));
		call_statistics.insert(std::make_pair(objectidentity, s));
		return s;
	}
	return i->second;
}

/**
 * \brief Return the number of calls to this  object
 *
 * This methods takes the sum over all counters
 */
unsigned long	CallStatistics::calls() const {
	unsigned long	result = 0;
	std::for_each(begin(), end(),
		[&result](const std::pair<std::string, unsigned long>& p) mutable {
			result += p.second;
		}
	);
	return result;
}

/**
 * \brief Return the numer of calls to a particular operation
 *
 * \param operation	name of the operation
 */
unsigned long	CallStatistics::calls(const std::string& operation) const {
	auto	i = find(operation);
	if (i == end()) {
		return 0;
	}
	return i->second;
}

/**
 * \brief Counte calls to an operation
 *
 * \param operation	name of the operation to count
 */
void	CallStatistics::count(const std::string& operation) {
	auto	i = find(operation);
	if (i == end()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "new operation %s on %s",
			operation.c_str(),
			Ice::identityToString(_objectidentity).c_str());
		insert(std::make_pair(operation, (unsigned long)1));
		return;
	}
	i->second++;
}

} // namespace snowstar
