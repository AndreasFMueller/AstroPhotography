/*
 * SxGuiderPort.h -- Starlight Express guider port
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SxGuiderPort_h
#define _SxGuiderPort_h

#include <AstroCamera.h>
#include <SxCamera.h>

namespace astro {
namespace camera {
namespace sx {

class SxGuiderPort : public GuiderPort {
	SxCamera&	camera;
public:
	SxGuiderPort(SxCamera& camera);
	virtual ~SxGuiderPort();
	virtual uint8_t	active();
	virtual void	activate(float raplus, float raminus,
		float decplus, float decminus);
};

} // namespace sx
} // namespace camera
} // namespace astro

#endif /* _SxGuiderPort_h */
