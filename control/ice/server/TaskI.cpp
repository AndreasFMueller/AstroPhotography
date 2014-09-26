/*
 * TaskI.cpp -- task servant implementaiton
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <TaskI.h>
#include <AstroTask.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/Communicator.h>
#include <TaskQueueI.h>
#include <TaskTable.h>
#include <ImageI.h>
#include <IceConversions.h>

namespace snowstar {

TaskI::TaskI(astro::persistence::Database& _database, long _queueid)
	: database(_database), queueid(_queueid) {
}

TaskI::~TaskI() {
}

astro::task::TaskQueueEntry	TaskI::entry() {
	astro::task::TaskTable	tasktable(database);
	if (!tasktable.exists(queueid)) {
		throw NotFound("task does not exist");
	}
	astro::task::TaskQueueEntry	result = tasktable.byid(queueid);
	return result;
}

TaskState	TaskI::state(const Ice::Current& /* current */) {
	return snowstar::convert(entry().state());
}

TaskParameters	TaskI::parameters(const Ice::Current& /* current */) {
	return snowstar::convert(entry().parameters());
}

TaskInfo	TaskI::info(const Ice::Current& /* current */) {
	return snowstar::convert(entry().info());
}

std::string	TaskI::imagename(const Ice::Current& /* current */) {
	return entry().filename();
}

ImagePrx	TaskI::getImage(const Ice::Current& /* current */) {
	std::string	filename = entry().filename();
	//return ImageI::createProxy(filename, current);
	return NULL;
}

} // namespace snowstar
