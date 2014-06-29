/*
 * GuidingProcess.h -- Base class for processes run by the guiding classes
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _GuidingProcess_h
#define _GuidingProcess_h

#include <AstroGuiding.h>
#include <AstroDebug.h>
#include <AstroPersistence.h>
#include <pthread.h>
#include <includes.h>
#include <Thread.h>

namespace astro {
namespace guiding {

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

private:
	persistence::Database	_database;
public:
	persistence::Database	database() { return _database; }

private:
	astro::thread::ThreadPtr	_thread;
public:
	astro::thread::ThreadPtr	thread() { return _thread; }
	void	thread(astro::thread::ThreadPtr t) { _thread = t; }

	void	stop();
	void	start();
	bool	wait(double timeout);
	bool	isrunning() { return _thread->isrunning(); }

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
	GuidingProcess(Guider& guider, TrackerPtr tracker,
		persistence::Database database = NULL);
	
};

} // namespace guiding
} // namespace astro

#endif /* _GuidingProcess_h */
