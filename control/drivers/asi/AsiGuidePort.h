/*
 * AisGuidePort.h -- ASI camera guider port
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AsiGuidePort_h
#define _AsiGuidePort_h

#include <AstroCamera.h>
#include <AsiCamera.hh>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace astro {
namespace camera {
namespace asi {

/**
 * \brief Implementation class for GuidePort on ASI cameras
 */
class AsiGuidePort : public GuidePort {
	AsiCamera&	_camera;
	std::mutex	_mutex;
	std::condition_variable	_condition;
	int	_ra;
	int	_dec;
	bool	_running;
	std::thread	*_thread;
	AsiGuidePort(const AsiGuidePort& other);
	AsiGuidePort&	operator=(const AsiGuidePort& other);
	void	north();
	void	south();
	void	east();
	void	west();
	void	rastop();
	void	decstop();
public:
	AsiGuidePort(AsiCamera& camera);
	virtual ~AsiGuidePort();
	virtual uint8_t	active();
	virtual void	activate(float raplus, float raminus,
				float decplus, float decminus);
	void	run();
};

} // namespace asi
} // namespace camera
} // namespace astro

#endif /* _AsiGuidePort_h */
