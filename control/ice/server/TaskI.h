/*
 * TaskI.h -- task servant declaration
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _TaskI_h
#define _TaskI_h

#include <tasks.h>
#include <AstroTask.h>
#include "StatisticsI.h"

namespace snowstar {

class TaskI : virtual public Task, public StatisticsI {
	astro::persistence::Database	database;
	long	queueid;
	astro::task::TaskQueueEntry	entry();
public:
	TaskI(astro::persistence::Database& database, long queueid);
	virtual ~TaskI();
	TaskState	state(const Ice::Current& current);
	TaskParameters	parameters(const Ice::Current& current);
	TaskInfo	info(const Ice::Current& current);
	std::string	imagename(const Ice::Current& current);
	ImagePrx	getImage(const Ice::Current& current);
	int	imageToRepo(const std::string& reponame,
			const Ice::Current& current);
};

} // namespace snowstar

#endif /* _TaskI_h */
