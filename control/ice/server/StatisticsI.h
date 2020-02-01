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

class CallStatistics : public std::map<std::string, unsigned long> {
	static std::map<std::string, CallStatisticsPtr>	call_statistics;
	std::string	_servantname;
public:
	CallStatistics(const std::string& servantname)
		: _servantname(servantname) {
	}

	static long	calls(const std::string& servantname,
				const std::string& callname);
	static void	count(const std::string& servantname,
				const std::string& callname);
	static std::list<std::string>	servantnames();
	static std::list<std::string>	callnames(
						const std::string& servantname);

	long	calls(const std::string& callname) {
		return calls(_servantname, callname);
	}

	void	count(const std::string& callname) {
		count(_servantname, callname);
	}

	static void	remember(const std::string& servantname,
				CallStatisticsPtr);
	static CallStatisticsPtr	recall(const std::string& servantname);
};


class StatisticsI : virtual public Daemon {
	CallStatisticsPtr	_statistics;
public:
	StatisticsI(const std::string& name);
	StatisticsI();
	~StatisticsI();
	InterfaceNameSequence	interfaceNames(const Ice::Current& current);
	long	servantInstances(const Ice::Current& current);
	ServantNameSequence	servantNames(const Ice::Current& current);
	long	interfaceCalls(const Ice::Current& current);
	long	interfaceNamedCalls(const std::string& callname,
			const Ice::Current& current);
	long	servantCalls(const std::string& servantname,
			const Ice::Current& current);
	long	servantNamedCalls(const std::string& servantname,
			const std::string& callname,
			const Ice::Current& current);
};

} // namespace snowstar

#endif /* _StatisticsI_h */
