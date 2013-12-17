/*
 * DrivingProcess.h -- thread driving the guider port during guiding
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _DrivingProcess_h
#define _DrivingProcess_h

#include <GuidingProcess.h>
#include <pthread.h>

namespace astro {
namespace guiding {

/**
 * \brief Driving process class
 */
class DrivingProcess : public GuidingProcess {
	pthread_mutex_t	mutex;
	/**
 	 * \brief Control interval for the port driving process
	 *
	 * The _interval variable is the time constant of the driving loop.
	 * Every _interval seconds the main loop checks the tx and ty 
	 * variables an redecides on the controls to be sent out the guider
	 * port.
	 */
	double	_interval;
public:
	const double&	interval() const { return _interval; }
	void	interval(const double& i);

private:
	/**
	 * \brief Speed control variables
	 *
	 * The variables tx and ty control the duty cycle with which the
	 * the guider port outputs are activated. They must be between
	 * -1 and 1, 0 meaning no activation. During the interval of
	 * length defined by the variable _interval, the ports will be
	 * activated for the time tx * _interval and ty * _interval
	 * respectively. Negative values mean that the raminus and decminus
	 * ports are activated, positive values activate the raplus and
	 * decplus outputs.
	 */
	double	tx, ty;
public:

	void	setCorrection(const double& _tx, const double& _ty);
private:
	// copy and assignment construtors are private to prevent copying
	DrivingProcess(const DrivingProcess& other);
	DrivingProcess&	operator=(const DrivingProcess& other);
public:
	DrivingProcess(Guider& _guider);
	~DrivingProcess();
	void	main(GuidingThread<DrivingProcess>& thread);
};

} // namespace guiding
} // namespace astro

#endif /* _DrivingProcess_h */
