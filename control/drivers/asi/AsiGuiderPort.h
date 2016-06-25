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

namespace astro {
namespace camera {
namespace asi {

class AsiGuiderPort : public GuiderPort {
	AsiCamera&	_camera;
	std::mutex	_mutex;
	std::condition_variable	_condition;
	float	_raplus;
	float	_raminus;
	float	_decplus;
	float	_decminus;
	AsiGuiderPort(const AsiGuiderPort& other);
	AsiGuiderPort&	operator=(const AsiGuiderPort& other);
public:
	AsiGuiderPort(AsiCamera& camera);
	virtual ~AsiGuiderPort();
	virtual uint8_t	active();
	virtual void	activate(float raplus, float raminus,
				float decplus, float decminus);
};

} // namespace asi
} // namespace camera
} // namespace astro

#endif /* _AsiGuiderPort_h */
