/*
 * GuiderProcess.h -- declaration of the GuiderProcess class
 *
 * This class is not to be exposed to applications, so we don't install
 * this header file
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _GuiderProcess_h
#define _GuiderProcess_h

#include <AstroGuiding.h>
#include <TrackingWork.h>
#include <DrivingWork.h>
#include <AstroPersistence.h>
#include <Thread.h>

using namespace astro::camera;

namespace astro {
namespace guiding {

/**
 * \brief Encapsulation of the guiding process
 */
class GuiderProcess {
	// common members
	Guider&	guider;
	TrackerPtr	tracker;

	// processes for tracking and driving
	DrivingWork	*drivingwork;
	TrackingWork	*trackingwork;

	astro::thread::ThreadPtr	tracking;
	astro::thread::ThreadPtr	driving;

	/**
	 * \brief Interval between images of the tracking thread
	 */
	double	_interval;
public:
	const double&	interval() const { return _interval; }
	void	interval(const double& i) { _interval = i; }
private:
	persistence::Database	database;
private:
	GuiderProcess(const GuiderProcess& other);
	GuiderProcess&	operator=(const GuiderProcess& other);
public:
	GuiderProcess(Guider& guider, double interval = 10,
		persistence::Database database = NULL);
	~GuiderProcess();
	bool	start(TrackerPtr tracker);
	bool	stop();
	bool	wait(double timeout);
	bool	isrunning() { return tracking->isrunning(); }
private:
	double	_gain;
public:
	double	getGain() const;
	void	setGain(double gain);

public:
	void lastAction(double& actiontime, Point& offset, Point& activation);
	const TrackingSummary&	summary();
};

} // namespace guiding
} // namespace astro

#endif /* _GuiderProcess_h */
