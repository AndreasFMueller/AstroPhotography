/*
 * SbigGuidePort.h -- SBIG guider port
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SbigGuidePort_h
#define _SbigGuidePort_h

#include <AstroCamera.h>
#include <SbigCamera.h>

namespace astro {
namespace camera {
namespace sbig {

class SbigGuidePort : public GuidePort {
	SbigCamera&	camera;
public:
	SbigGuidePort(SbigCamera& camera);
	virtual ~SbigGuidePort();
	virtual uint8_t	active();
	virtual void	activate(float raplus, float raminus,
		float decplus, float decminus);
};

} // namespace sbig
} // namespace camera
} // namespace astro

#endif /* _SbigGuidePort_h */
