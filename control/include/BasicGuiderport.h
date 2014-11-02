/*
 * BasicGuiderport.h -- interface class for Basic guiderports
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswi
 */
#ifndef _BasicGuiderport_h
#define _BasicGuiderport_h

#include <AstroDevice.h>
#include <AstroCamera.h>
#include <Serial.h>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <astrochrono.h>

namespace astro {
namespace camera {

/**
 * \brief Basic guiderport framework
 *
 * Hardware devices that work as a guider port can be controlled by this
 * class. It provides all the timeing, the only thing that needs to be
 * implemented in a derived class is the method do_activate which actually
 * activates the output pins of the particular hardware.
 */
class BasicGuiderport : public astro::camera::GuiderPort {
	chrono::steady_clock::time_point	nextchange[4];
	volatile bool	_running;
	volatile uint8_t	_active;
public:
	bool	running() const { return _running; }
private:
	std::mutex		mtx;
	std::condition_variable	cond;
	std::thread		thread;
public:
	BasicGuiderport(const std::string& devicename);
	virtual ~BasicGuiderport();

	// accessors
	virtual uint8_t	active();
	virtual void	activate(float raplus, float raminus,
				float declus, float decminus);

	// the run method for the guiderport
	virtual void	do_activate(uint8_t active);
	void	run();
	void	stop();
	void	start();
};

} // namespace camera
} // namespace astro

#endif /* _BasicGuiderport */
