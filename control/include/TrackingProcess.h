/*
 * TrackingProcess.h -- thread handling the camera during guiding
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _TrackingProcess_h
#define _TrackingProcess_h

#include <GuidingProcess.h>
#include <DrivingProcess.h>

namespace astro {
namespace guiding {

/**
 * \brief Tracking process class
 *
 * Autoguiding uses two threads. One monitors the guide star in the camera,
 * computes corrections and sends them to the second thread. The second
 * thread then drives the guider port. Some cameras may actually be able
 * to accept guider port commands while exposing an image. In such cases,
 * the time constants controlling both process may be different. In many
 * cases, however, it will be necessary to use the same time constant and
 * the threads that are designed to be independent actually become 
 * synchronized.
 */
class TrackingProcess : public GuidingProcess {
	/**
	 * \brief 
	 *
	 * The _gain variable increases the amount of correction sent to the
	 * guider port so that sluggish mounts can moved more quickly. It
	 * normally should not be different from 1, as that means that the
	 * correction brings the mount into exact alignment by the end of
	 * the next correction interval.
	 */
	double	_gain;
public:
	const double&	gain() const { return _gain; }
	void	gain(const double& g) { _gain = g; }
private:
	/**
	 * \brief Time constant for the tracking loop
	 *
	 * The tracking loop takes a tracking image every time it traverses
	 * the loop.
	 */
	double	_interval;
public:
	const double&	interval() const { return _interval; }
	void	interval(const double& i);
private:
	/**
	 * \brief The process to drive
	 *
	 * The tracking loop needs a place to send the guiding commands. 
	 * This is the driving process. It implements a method setCorrection
	 * that takes the guider port activation duty cycle data
	 */
	DrivingProcess&	_drivingprocess;
private:
	// prevent copy
	TrackingProcess(const TrackingProcess& other);
	TrackingProcess&	operator=(const TrackingProcess& other);
public:
	TrackingProcess(Guider& _guider, TrackerPtr _tracker,
		DrivingProcess& drivingprocess);
	~TrackingProcess();

	void	main(GuidingThread<TrackingProcess>& thread);
};

} // namespace guiding
} // namespace astro

#endif /* _TrackingProcess_h */
