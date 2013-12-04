/*
 * Task_impl.h -- CORBA task servant
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Task_impl_h
#define _Task_impl_h

#include <tasks.hh>
#include <AstroTask.h>
#include <TaskTable.h>

namespace Astro {

/**
 * \brief Task servant definition
 */
class Task_impl : public virtual POA_Astro::Task {
	astro::task::TaskTable&	_tasktable;
	long	_queueid;
	astro::task::TaskQueueEntry	entry();
public:
	long	queueid() const { return _queueid; }
	Task_impl(astro::task::TaskTable& tasktable, long queueid);
	virtual TaskState	state();
	virtual TaskInfo	*info();
	virtual TaskParameters	*parameters();
	virtual Image_ptr	getImage();
	virtual char	*imagename();
};

} // namespace Astro

#endif /* _Task_impl_h */
