/*
 * StatisticsI.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "StatisticsI.h"
#include <AstroFormat.h>
#include <AstroDebug.h>

namespace snowstar {

StatisticsI::StatisticsI() {
}

StatisticsI::~StatisticsI() {
}

/**
 * \brief Return a list of object identities
 */
ObjectIdentitySequence   StatisticsI::objectidentities(
	const Ice::Current& current) {
	CallStatistics::count(current);
	std::list<Ice::Identity>	identities
		= CallStatistics::objectidentities();
	ObjectIdentitySequence	result;
	std::copy(identities.begin(), identities.end(),
		std::back_inserter(result));
	return result;
}

/**
 * \brief Return the number of objects known to the server
 */
Ice::Long    StatisticsI::objectidentityCount(const Ice::Current& current) {
	CallStatistics::count(current);
	return CallStatistics::objectidentityCount();
}

/**
 * \brief Get a list of operations
 */
OperationSequence	StatisticsI::operations(
				const Ice::Identity& objectidentity,
				const Ice::Current& current) {
	CallStatistics::count(current);
	std::list<std::string>	o = CallStatistics::operations(objectidentity);
	OperationSequence	result;
	std::copy(o.begin(), o.end(), std::back_inserter(result));
	return result;
}

/**
 * \brief Return the number of operation count
 */
Ice::Long	StatisticsI::operationCount(const Ice::Identity& objectidentity,
				const Ice::Current& current) {
	CallStatistics::count(current);
	return CallStatistics::operationCount(objectidentity);
}

/**
 * \brief 
 */
Ice::Long	StatisticsI::callsPerObject(const Ice::Identity& objectidentity,
		const Ice::Current& current) {
	CallStatistics::count(current);
	return CallStatistics::calls(objectidentity);
}

/**
 * \brief
 */
Ice::Long	StatisticsI::callsPerObjectAndOperation(
		const Ice::Identity& objectidentity,
		const std::string& operation,
		const Ice::Current& current) {
	CallStatistics::count(current);
	return CallStatistics::calls(objectidentity, operation);
}

/**
 * \brief Return the total number of alls on this object
 */
Ice::Long	StatisticsI::calls(const Ice::Current& current) {
	CallStatistics::count(current);
	return CallStatistics::recall(current.id)->calls();
}

/**
 * \brief Return the number of calls to an operation on this object
 */
Ice::Long	StatisticsI::operationCalls(const std::string& operation,
		const Ice::Current& current) {
	CallStatistics::count(current);
	return CallStatistics::recall(current.id)->calls(operation);
}

} // namespace snowstar
