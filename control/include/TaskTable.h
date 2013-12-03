/*
 * TaskTable.h -- Table containing Task entries
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _TaskTable_h
#define _TaskTable_h

#include <AstroTask.h>
#include <AstroPersistence.h>

namespace astro {
namespace task {

/**
 * \brief Task table adapter
 */
class TaskTableAdapter {
public:
static std::string	tablename();
static std::string	createstatement();
static TaskQueueEntry
	row_to_object(int objectid, const astro::persistence::Row& row);
static astro::persistence::UpdateSpec
	object_to_updatespec(const TaskQueueEntry& entry);
};

/**
 * \brief A table built with the TaskTableAdapter is a TaskTable
 */
typedef astro::persistence::Table<TaskQueueEntry, TaskTableAdapter> TaskTable;

} // namespace task
} // namespace astro

#endif /* _TaskTable_h */

