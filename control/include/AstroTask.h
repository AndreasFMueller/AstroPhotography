/*
 * AstroTask.h -- task objects, contain all the information for a task
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroTask_h
#define _AstroTask_h

#include <AstroCamera.h>

namespace astro {
namespace task {

/**
 * \brief Task object
 *
 */
class Task {
private:
	astro::camera::Exposure	_exposure;
public:
	const astro::camera::Exposure&	exposure() const { return _exposure; }
	void	exposure(const astro::camera::Exposure& exposure) {
		_exposure = exposure;
	}

private:
	std::string	_camera;
public:
	const std::string&	camera() const { return _camera; }
	void	camera(const std::string& camera) { _camera = camera; }

private:
	long	_ccdid;
public:
	long	ccdid() const { return _ccdid; }
	void	ccdid(long ccdid) { _ccdid = ccdid; }

private:
	float	_ccdtemperature;
public:
	float	ccdtemperature() const { return _ccdtemperature; }
	void	ccdtemperature(float ccdtemperature) {
			_ccdtemperature = ccdtemperature;
	}

private:
	std::string	_filterwheel;
public:
	const std::string&	filterwheel() const { return _filterwheel; }
	void	filterwheel(const std::string& filterwheel) {
			_filterwheel = filterwheel;
	}

private:
	long	_filterposition;
public:
	long	filterposition() const { return _filterposition; }
	void	filterposition(long filterposition) {
			_filterposition = filterposition;
	}

	Task();
};

/**
 * \brief Task Queue entry
 */
class TaskQueueEntry : public Task {
private:
	long	_id;
public:
	const long	id() const { return _id; }

public:
	typedef enum { pending, executing, failed, cancelled, complete } taskstate;
private:
	taskstate	_state;
public:
	taskstate	state() const { return _state; }
	void	state(taskstate state) { _state = state; }

private:
	std::string	_filename;
public:
	const std::string& filename() const { return _filename; }
	void	filename(const std::string& filename) { _filename = filename; }

	TaskQueueEntry(const Task& task);
public:

	// find out whether a this task blocks some other task
	bool	blocks(const TaskQueueEntry& other) const;
	bool	blockedby(const TaskQueueEntry& other) const;
};

class	TaskExecutor;
typedef std::shared_ptr<TaskExecutor>	TaskExecutorPtr;

/**
 * \brief Task queue object
 */
class TaskQueue {
	typedef std::map<int id, TaskExecutorPtr>	executormap;
	executormap	executors;
	pthread_t	_thread;
private:
	// prevent copying
	TaskQueue(const TaskQueue& other);
	TaskQueue&	operator=(const TaskQueue& other);
public:
	TaskQueue();
	~TaskQueue();
	void	terminate(int taskid);
	void	callback(const TaskQueueEntry& entry);
	void	update(const TaskQueueEntry& entry);
};

/**
 * \brief TaskExecutor
 */
class TaskExecutor {
	TaskQueue	_queue;
	TaskQueueEntry	_task;
	pthread_t	_thread;
private:
	// ensure that the TaskExecutor cannot be copied
	operator=(const TaskExecutor& other);
	TaskExecutor(const TaskExecutor& other);
public:
	TaskExecutor(TaskQueueEntry& task);
	~TaskExecutor();

	void	main();
};

} // namespace task
} // namespace astro

#endif /* _AstroTask_h */
