/*
 * SxGuiderPort.h -- Starlight Express guider port
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SxGuiderPort_h
#define _SxGuiderPort_h

#include <AstroCamera.h>
#include <SxCamera.h>
#include <pthread.h>
#include <time.h>

namespace astro {
namespace camera {
namespace sx {

class timespec {
	void	normalize();
public:
	struct ::timespec	ts;
	timespec();
	timespec(struct ::timespec& when);
	timespec(struct timeval& when);
	timespec(double when);
	timespec(const timespec& other);
	timespec	operator+(const timespec& other) const;
	timespec	operator+(const double& other) const;
	bool	operator<(const timespec& other) const;
	std::string	toString() const;
};

class SxGuiderPort : public GuiderPort {
	SxCamera&	camera;
	std::vector<timespec>	turnoff;
	// current state 
	uint8_t	current;
	// worker thread
	pthread_t	thread;
	// Condition variable for signaling a state change to the worker thread.
	pthread_cond_t	condition;
	// Mutex to protect the request variable
	pthread_mutex_t	mutex;
	bool	cancel;
public:
	SxGuiderPort(SxCamera& camera);
	virtual ~SxGuiderPort();
	virtual uint8_t	active();
	virtual void	activate(float raplus, float raminus,
		float decplus, float decminus);
	void	*main();
};

} // namespace sx
} // namespace camera
} // namespace astro

#endif /* _SxGuiderPort_h */
