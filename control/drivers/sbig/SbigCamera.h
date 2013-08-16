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

/**
 * \brief SBIG camera object
 */
class SbigCamera : public Camera {
	unsigned short	cameraType;
	short	handle;
	void	sethandle();
public:
	SbigCamera(int usbno);
	virtual ~SbigCamera();
protected:
	virtual CcdPtr	getCcd0(size_t id);
	virtual FilterWheelPtr	getFilterWheel0() throw (not_implemented);
	virtual GuiderPortPtr	getGuiderPort0() throw (not_implemented);
	friend class SbigCcd;
	friend class SbigFilterWheel;
	friend class SbigCooler;
	friend class SbigGuiderPort;
};

} // namespace sbig
} // namespace camera
} // namespace astro

#endif /* _SbigCamera_h */
