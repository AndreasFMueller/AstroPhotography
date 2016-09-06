/*
 * BasicProcess.h -- Base class for processes run by the guiding classes
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _BasicProcess_h
#define _BasicProcess_h

#include <AstroGuiding.h>
#include <AstroDebug.h>
#include <AstroPersistence.h>
#include <AstroUtils.h>
#include <includes.h>

namespace astro {
namespace guiding {

/**
 * \brief Guiding Process base class
 */
class BasicProcess {
	GuiderBase	*_guider;
public:
	bool	hasGuider() const;
	GuiderBase	*guider();

protected:
	double	_focallength;
public:
	double	focallength() const { return _focallength; }
	void	focallength(double f) { _focallength = f; }

	// everything needed for all processes, i.e. exposure parameters,
	// imager object (which encapsulates the ccd), 
private:
	camera::Exposure	_exposure;
	camera::Imager&		_imager;
public:
	const camera::Exposure&	exposure() const { return _exposure; }
	camera::Imager&	imager() { return _imager; }

	// the tracker is needed to locate the a star
private:
	TrackerPtr	_tracker;
public:
	TrackerPtr	tracker() { return _tracker; }
	const TrackerPtr	tracker() const { return _tracker; }

	// the database used used for persistence
private:
	persistence::Database	_database;
public:
	persistence::Database	database() { return _database; }

	// each process has a thread assocated with it
private:
	thread::ThreadPtr	_thread;
public:
	thread::ThreadPtr	thread() { return _thread; }
	void	thread(thread::ThreadPtr t) { _thread = t; }

	void	cancel();
	void	stop();
	virtual void	start();
	bool	wait(double timeout);
	bool	isrunning() { return _thread->isrunning(); }

	// constructors for the basic process
public:
	BasicProcess(GuiderBase *guider, TrackerPtr tracker,
		persistence::Database database = NULL);
	BasicProcess(const camera::Exposure& exposure, camera::Imager& imager,
		TrackerPtr tracker, persistence::Database database = NULL);
};

} // namespace guiding
} // namespace astro

#endif /* _BasicProcess_h */
