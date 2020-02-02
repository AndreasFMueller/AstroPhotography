/*
 * StatisticsI.h
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _StatisticsI_h
#define _StatisticsI_h

#include <types.h>
#include <map>
#include <string>
#include <list>
#include <memory>

namespace snowstar {

class CallStatistics;
typedef std::shared_ptr<CallStatistics>	CallStatisticsPtr;

/**
 * \brief A container class for call statistics information
 */
class CallStatistics : public std::map<std::string, unsigned long> {
	static std::map<Ice::Identity, CallStatisticsPtr>	call_statistics;
	Ice::Identity	_objectidentity;
public:
	CallStatistics(const Ice::Identity& objectidentity)
		: _objectidentity(objectidentity) {
	}

	// return information on object ids
	static std::list<Ice::Identity>	objectidentities();
	static unsigned long	objectidentityCount();

	// return information on objects
	static std::list<std::string>	operations(
					const Ice::Identity& objectidentity);
	static unsigned long	operationCount(
					const Ice::Identity& objectidentity);

	// return various counters
	static unsigned long	calls(const Ice::Identity& objectidentity);
	static unsigned long	calls(const Ice::Identity& objectidentity,
					const std::string& operation);

	// count a call to an operation
	static void	count(const Ice::Identity& objectidentity,
					const std::string& operation);
	static void	count(const Ice::Current& current);

	// return number of calls
	unsigned long	calls(const std::string& operation) const;
	unsigned long	calls() const;
	void	count(const std::string& operation);

	// access to the call statistics objects
	static CallStatisticsPtr	recall(
					const Ice::Identity& objectidentity);
};

/*
 * Implementation of the statistics interface inherited by many
 */
class StatisticsI : virtual public Statistics {
public:
	StatisticsI();
	~StatisticsI();
	// object identities
	ObjectIdentitySequence	objectidentities(const Ice::Current& current);
	Ice::Long	objectidentityCount(const Ice::Current& current);
	// operations
	OperationSequence	operations(const Ice::Identity& objectidentity,
					const Ice::Current& current);
	Ice::Long	operationCount(const Ice::Identity& objectidentity,
					const Ice::Current& current);

	// global statistics
	Ice::Long	callsPerObject(const Ice::Identity& objectidentity,
			const Ice::Current& current);
	Ice::Long	callsPerObjectAndOperation(const Ice::Identity& objectidentity,
			const std::string& operation,
			const Ice::Current& current);
	
	// operations for this instance only, uses the object identity in
	// the current argument
	Ice::Long	calls(const Ice::Current& current);
	Ice::Long	operationCalls(const std::string& operation,
			const Ice::Current& current);
};

} // namespace snowstar

#endif /* _StatisticsI_h */
