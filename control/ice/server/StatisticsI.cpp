/*
 * StatisticsI.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "StatisticsI.h"
#include <AstroFormat.h>

namespace snowstar {

StatisticsI::StatisticsI(const std::string& name) {
	CallStatisticsPtr	ptr(new CallStatistics(name));
	CallStatistics::remember(name, ptr);
}

StatisticsI::StatisticsI() {
	std::string	name = astro::stringprintf("%p", this);
	CallStatisticsPtr	ptr(new CallStatistics(name));
	CallStatistics::remember(name, ptr);
}

StatisticsI::~StatisticsI() {
}

InterfaceNameSequence   StatisticsI::interfaceNames(const Ice::Current& current) {
	InterfaceNameSequence	result;
	// XXX implementation missing
	return result;
}

long    StatisticsI::servantInstances(const Ice::Current& current) {
	return 0;
}

ServantNameSequence     StatisticsI::servantNames(const Ice::Current& current) {
	ServantNameSequence	result;
	// XXX implementation missing
	return result;
}

long    StatisticsI::interfaceCalls(const Ice::Current& current) {
	// XXX implementation missing
	return 0;
}

long    StatisticsI::interfaceNamedCalls(const std::string& callname,
		const Ice::Current& current) {
	// XXX implementation missing
	return 0;
}

long    StatisticsI::servantCalls(const std::string& servantname,
		const Ice::Current& current) {
	// XXX implementation missing
	return 0;
}

long    StatisticsI::servantNamedCalls(const std::string& servantname,
		const std::string& callname,
		const Ice::Current& current) {
	// XXX implementation missing
	return 0;
}

} // namespace snowstar
