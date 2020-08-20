/*
 * OthelloFocuser.h -- Othello focuser hardware definitions
 *
 * (c) 2016 prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _OthelloFocuser_h
#define _OthelloFocuser_h

#include <AstroCamera.h>
#include <AstroUSB.h>
#include <thread>

namespace astro {
namespace camera {
namespace othello {

class OthelloFocuser : public astro::camera::Focuser {
	astro::usb::DevicePtr	deviceptr;
	std::recursive_mutex	_mutex;
	std::condition_variable_any	_condition;
	std::thread	_thread;
	OthelloFocuser(const OthelloFocuser& other);
	OthelloFocuser&	operator=(const OthelloFocuser& other);
	long	_current;
	bool	_running;
	static void	main(OthelloFocuser *focuser) noexcept;
	void	run();
	void	stop();
	void	start();
public:
	OthelloFocuser(astro::usb::DevicePtr _deviceptr);
	~OthelloFocuser();
	virtual long	min();
	virtual long	max();
	virtual long	current();
	virtual void	set(long value);
};

} // namespace othello
} // namespace camera
} // namespace astro

#endif /* _OthelloFocuser_h */
