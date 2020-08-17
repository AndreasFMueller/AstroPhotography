/*
 * TaskInfo.cpp -- TaskInfo implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTask.h>
#include <stdexcept>
#include <AstroFormat.h>

namespace astro {
namespace task {

TaskInfo::TaskInfo(long id) : _id(id) {
}

void	TaskInfo::now() {
	time_t	t = time(NULL);
	lastchange(t);
}

std::string      TaskInfo::state2string(taskstate t) {
	switch (t) {
	case pending:
		return std::string("pending");
		break;
	case executing:
		return std::string("executing");
		break;
	case failed:
		return std::string("failed");
		break;
	case cancelled:
		return std::string("cancelled");
		break;
	case complete:
		return std::string("complete");
		break;
	case deleted:
		return std::string("deleted");
		break;
	}
	throw std::runtime_error("unknown task state code");
}

TaskInfo::taskstate        TaskInfo::string2state(const std::string& s) {
	if (s == "pending") {
		return pending;
	}
	if (s == "executing") {
		return executing;
	}
	if (s == "failed") {
		return failed;
	}
	if (s == "cancelled") {
		return cancelled;
	}
	if (s == "complete") {
		return complete;
	}
	if (s == "deleted") {
		return deleted;
	}
	throw std::runtime_error("unknown task state name");
}

std::string	TaskInfo::toString() const {
	std::string	s = stringprintf("task[%d] %s %s %s %s",
		id(), state2string(state()).c_str(),
		frame().toString().c_str(), filename().c_str(),
		cause().c_str());
	return s;
}

} // namespace task
} // namespace astro
