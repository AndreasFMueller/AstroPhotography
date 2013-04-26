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

class SbigCamera : public Camera {
	unsigned short	cameraType;
	short	handle;
	void	sethandle();
public:
	SbigCamera();
	virtual ~SbigCamera();
	virtual CcdPtr	getCcd(int id);
	friend class SbigCcd;
};

} // namespace sbig
} // namespace camera
} // namespace astro

#endif /* _SbigCamera_h */
