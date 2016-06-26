/*
 * AisGuiderPort.h -- ASI camera guider port
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AsiGuiderPort_h
#define _AsiGuiderPort_h

#include <AstroCamera.h>
#include <AsiCamera.hh>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace astro {
namespace camera {
namespace asi {

/**
 * \brief Implementation class for GuiderPort on ASI cameras
 */
class AsiGuiderPort : public GuiderPort {
	AsiCamera&	_camera;
	std::mutex	_mutex;
	std::condition_variable	_condition;
	int	_ra;
	int	_dec;
	bool	_running;
	std::thread	*_thread;
	AsiGuiderPort(const AsiGuiderPort& other);
	AsiGuiderPort&	operator=(const AsiGuiderPort& other);
	void	north();
	void	south();
	void	east();
	void	west();
	void	rastop();
	void	decstop();
public:
	AsiGuiderPort(AsiCamera& camera);
	virtual ~AsiGuiderPort();
	virtual uint8_t	active();
	virtual void	activate(float raplus, float raminus,
				float decplus, float decminus);
	void	run();
};

} // namespace asi
} // namespace camera
} // namespace astro

#endif /* _AsiGuiderPort_h */
