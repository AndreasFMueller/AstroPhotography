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
 * \brief Exposure Task class
 *
 * This class encapsulates what the executor has to do
 */
class ExposureWork : public CancellableWork {
	astro::camera::CameraPtr        camera;
	astro::camera::CcdPtr           ccd;
	astro::camera::CoolerPtr        cooler;
	astro::camera::FilterWheelPtr   filterwheel;
	TaskQueueEntry& _task;
public:
	ExposureWork(TaskQueueEntry& task);
	~ExposureWork();
	virtual void    run();
};

} // namespace task
} // namespace astro

#endif /* _ExposureWork_h */
