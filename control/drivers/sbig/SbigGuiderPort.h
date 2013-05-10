/*
 * SbigGuiderPort.h -- SBIG guider port
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SbigGuiderPort_h
#define _SbigGuiderPort_h

#include <AstroCamera.h>
#include <SbigCamera.h>

namespace astro {
namespace camera {
namespace sbig {

class SbigGuiderPort : public GuiderPort {
	SbigCamera&	camera;
public:
	SbigGuiderPort(SbigCamera& camera);
	virtual ~SbigGuiderPort();
	virtual uint8_t	active();
	virtual void	activate(float raplus, float raminus,
		float decplus, float decminus);
};

} // namespace sbig
} // namespace camera
} // namespace astro

#endif /* _SbigGuiderPort_h */
