/*
 * ExposureWork.h -- Work to be done for exposure task
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ExposureWork_h
#define _ExposureWork_h

#include <AstroTask.h>
#include <CancellableWork.h>

namespace astro {
namespace task {

/**
 * \brief Base class for all tasks
 */
class TaskWork : public CancellableWork {
	TaskQueueEntry& _task;
public:
	TaskQueueEntry&	task() { return _task; }
	TaskWork(TaskQueueEntry& task) : _task(task) { }
	~TaskWork() { }
};

/**
 * \brief Exposure Task class
 *
 * This class encapsulates what the executor has to do
 */
class ExposureWork : public TaskWork {
	astro::camera::CameraPtr        camera;
	astro::camera::CcdPtr           ccd;
	astro::camera::CoolerPtr        cooler;
	astro::camera::FilterWheelPtr   filterwheel;
	astro::device::MountPtr   	mount;
	astro::camera::FocuserPtr   	focuser;
public:
	ExposureWork(TaskQueueEntry& task);
	~ExposureWork();
	virtual void    run();
};

/**
 * \brief SleepWork task class
 *
 * This class encapsulates what the executor has to do for a sleep task
 */
class SleepWork : public TaskWork {
	double	_sleeptime;
public:
	SleepWork(TaskQueueEntry& task);
	~SleepWork();
	virtual void	run();
};

/**
 * \brief SleepWork task class
 *
 * This class encapsulates what the executor has to do for a dither task
 */
class DitherWork : public TaskWork {
public:
	DitherWork(TaskQueueEntry& task);
	~DitherWork();
	virtual void	run();
};

} // namespace task
} // namespace astro

#endif /* _ExposureWork_h */
