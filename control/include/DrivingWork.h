/*
 * DrivingWork.h -- thread driving the guider port during guiding
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _DrivingWork_h
#define _DrivingWork_h

#include <GuidingProcess.h>
#include <mutex>

namespace astro {
namespace guiding {

/**
 * \brief Driving process class
 *
 * The driving process is responsible for applying the the correction to
 * the guider port. As a derived class of GuidingProcess, it always has
 * a GuiderPort reference available. The correction is applied using the
 * setCorrection method.
 */
class DrivingWork : public GuidingProcess {
	std::mutex	mutex;
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
	 * \brief When the correction started
	 */
	double	correctionstart;
	/**
	 * \brief Offset control variables
	 *
	 * The variables totalx and totaly indicate the total time the
	 * corresponding guider ports have to be activated. Usually, one
	 * will want to perform the correction as quickly as possible.
	 */
	double	stepx, stepy;
	double	totalx, totaly;
public:
	void	setCorrection(const double& _tx, const double& _ty);

	/**
	 * \brief Default correction
	 *
	 * This is the correction to be applied constantly to compensate
	 * for any drift
	 */
	double	defaultx, defaulty;
public:
	void	defaultCorrection(const double& _tx, const double& _ty);
	
private:
	// copy and assignment construtors are private to prevent copying
	DrivingWork(const DrivingWork& other);
	DrivingWork&	operator=(const DrivingWork& other);
public:
	DrivingWork(Guider& _guider);
	~DrivingWork();

	void	main(astro::thread::Thread<DrivingWork>& thread);
};

} // namespace guiding
} // namespace astro

#endif /* _DrivingWork_h */
