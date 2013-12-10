/*
 * GuidingProcess.h -- Base class for processes run by the guiding classes
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _GuidingProcess_h
#define _GuidingProcess_h

#include <AstroGuiding.h>
#include <pthread.h>

namespace astro {
namespace guiding {

/**
 * \brief Locking class to make locking more automatic
 */
class GuidingLock {
	pthread_mutex_t	*_mutex;
public:
	GuidingLock(pthread_mutex_t *mutex) : _mutex(mutex) {
		if (pthread_mutex_lock(_mutex)) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "locking fails: %s",
				strerror(errno));
		}
	}
	~GuidingLock() {
		if (pthread_mutex_unlock(_mutex)) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "");
		}
	}
};

class MainAccess;

/**
 * \brief Class encapsulating the thread stuff
 */
class ThreadBase {
	pthread_t	thread;
private:
	pthread_cond_t	waitcond; // waiting
	pthread_mutex_t	mutex;
protected:
	volatile bool	_isrunning;
public:
	bool	isrunning();
protected:
	virtual	void	main() = 0;
private:
	// The MainAccess adapter class gives access allows to get access
	// to the protected main function from a C function
	friend class RunAccess;
	void	run();

	// signaling boolean variable that can be use to signal the thread
	// to terminate
private:
	volatile bool	_terminate;
public:
	bool	terminate() const { return _terminate; }
private:	// prevent copying of this object
	ThreadBase(ThreadBase& other);
	ThreadBase&	operator=(ThreadBase& other);
public:
	ThreadBase();
	~ThreadBase();
	void	start();
	void	stop();
public:
	bool	wait(double timeout);
};
typedef std::shared_ptr<ThreadBase>	ThreadPtr;

/**
 * \brief Generic thread template
 *
 * This template implements the "mechanical" aspects of a thread. The work
 * is encapsulated in the Work template argument. A Work class must implement
 * the main method, which is called with the thread object as argument
 * The main function can use the thread object for synchronization and for
 * signaling.
 */
template<typename Work>
class GuidingThread : public ThreadBase {
	Work&	_work;
public:
	GuidingThread(Work& work) : _work(work) { }
	virtual void	main() {
		_work.main(*this);
	}
};

/**
 * \brief Guiding Process base class
 */
class GuidingProcess {
	Guider&		_guider;
public:
	Guider&	guider() { return _guider; }
	const Guider&	guider() const { return _guider; }
private:
	TrackerPtr	_tracker;
public:
	TrackerPtr	tracker() { return _tracker; }
	const TrackerPtr	tracker() const { return _tracker; }
	// simplified accessors for the stuff needed during calibration or
	// guiding
	astro::camera::Exposure&	exposure() {
		return _guider.exposure();
	}
	const astro::camera::Exposure&	exposure() const {
		return _guider.exposure();
	}
	astro::camera::GuiderPortPtr	guiderport() {
		return _guider.guiderport();
	}
	const astro::camera::GuiderPortPtr	guiderport() const {
		return _guider.guiderport();
	}
	astro::camera::Imager&		imager() {
		return _guider.imager();
	}
	const astro::camera::Imager&		imager() const {
		return _guider.imager();
	}
	GuiderCalibration&	calibration() {
		return _guider.calibration();
	}
	const GuiderCalibration&	calibration() const {
		return _guider.calibration();
	}

public:
	GuidingProcess(Guider& guider, TrackerPtr tracker);
	
	virtual void	start() = 0;
	virtual void	stop() = 0;
};

} // namespace guiding
} // namespace astro

#endif /* _GuidingProcess_h */
