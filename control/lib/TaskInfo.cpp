/*
 * TaskInfo.cpp -- TaskInfo implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTask.h>

namespace astro {
namespace task {

TaskInfo::TaskInfo(long id) : _id(id) {
}

void	TaskInfo::now() {
	time_t	t = time(NULL);
	lastchange(t);
}

} // namespace task
} // namespace astro
