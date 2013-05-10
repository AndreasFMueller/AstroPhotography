/*
 * SbigCamera.h -- SBIG camera abstraction
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SbigCamera_h
#define _SbigCamera_h

#include <AstroCamera.h>

using namespace astro::camera;

namespace astro {
namespace camera {
namespace sbig {

class SbigCcd;
class SbigFilterWheel;
class SbigCooler;
class SbigGuiderPort;

class SbigCamera : public Camera {
	unsigned short	cameraType;
	short	handle;
	void	sethandle();
public:
	SbigCamera();
	virtual ~SbigCamera();
	virtual CcdPtr	getCcd(size_t id);
	virtual FilterWheelPtr	getFilterWheel() throw (not_implemented);
	virtual GuiderPortPtr	getGuiderPort() throw (not_implemented);
	friend class SbigCcd;
	friend class SbigFilterWheel;
	friend class SbigCooler;
	friend class SbigGuiderPort;
};

} // namespace sbig
} // namespace camera
} // namespace astro

#endif /* _SbigCamera_h */
