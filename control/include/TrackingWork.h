/*
 * TrackingWork.h -- thread handling the camera during guiding
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _TrackingWork_h
#define _TrackingWork_h

#include <GuidingProcess.h>
#include <DrivingWork.h>
#include <deque>

namespace astro {
namespace guiding {

/**
 * \brief Entries in the tracking history
 */
typedef	std::pair<double, Point>	trackinghistoryentry;

/**
 * \brief The tracking history is a double ended queue
 */
typedef std::deque<trackinghistoryentry>	trackinghistory_type;

std::ostream&	operator<<(std::ostream& out,
	const trackinghistoryentry& entry);

std::string	toString(const trackinghistoryentry& entry);

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
class TrackingWork : public GuidingProcess {
	trackinghistory_type	trackinghistory;
	void	addHistory(const Point& point);
public:
	void	dumpHistory(std::ostream& out);

	/**
	 * \brief length of the tracking history to keep
	 *
	 * The Work class keeps a history of tracking offsets. To keep 
	 * memory consumption low during long tracking runs, only the
	 * most recent tracking points are kept, their number is limited by
	 * the _history_length. The value -1 turns the tracking history off
	 * completely.
	 */
	long	_history_length;
public:
	const long&	history_length() const {
		return _history_length; }
	void	history_length(const long l) { _history_length = l; }
private:
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
	DrivingWork&	_driving;
private:
	// prevent copy
	TrackingWork(const TrackingWork& other);
	TrackingWork&	operator=(const TrackingWork& other);
public:
	TrackingWork(Guider& _guider, TrackerPtr _tracker,
		DrivingWork& driving);
	~TrackingWork();

	void	main(GuidingThread<TrackingWork>& thread);
private:
	double	_lastaction;
	Point	_offset;
	Point	_activation;
public:
	void	lastAction(double& actiontime, Point& offset,
			Point& activation);
};

} // namespace guiding
} // namespace astro

#endif /* _TrackingWork_h */
