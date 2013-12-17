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
#include <pthread.h>

using namespace astro::camera;

namespace astro {
namespace guiding {

/**
 * \brief Encapsulation of the guiding process
 */
class GuiderProcess {
	// guiding: handles the guider port
	typedef struct thread_s {
		pthread_attr_t	attr;
		pthread_t	thread;
		pthread_cond_t	cond;
		pthread_mutex_t	mutex;
	} thread_t;
	thread_t	guide;

	// tracking: handles the tracker
	bool	tracking;
	pthread_attr_t	trackattr;
	pthread_t	track;
	
	// 
	pthread_mutex_t	mutex; // protects tx, ty variables
	double	tx, ty;
	double	gain;

	// common members
	Guider&	guider;
	TrackerPtr	tracker;

	// Interval between images
	double	_interval;
public:
	const double&	interval() const { return _interval; }
	void	interval(const double& i) { _interval = i; }

public:
	// main functions
	void	*guide_main();
	void	*track_main();
public:
	GuiderProcess(Guider& guider, double interval = 10);
	~GuiderProcess();
	bool	start(TrackerPtr tracker);
	bool	stop();
	bool	wait(double timeout);
	double	getGain() const;
	void	setGain(double gain);
};

} // namespace guiding
} // namespace astro

#endif /* _GuiderProcess_h */
