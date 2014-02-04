/*
 * CameraI.h -- ICE camera wrapper definition
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _CameraI_h
#define _CameraI_h

#include <camera.h>
#include <AstroCamera.h>

namespace snowstar {

class CameraI : public Camera {
	astro::camera::CameraPtr	_camera;
public:
	CameraI(astro::camera::CameraPtr camera);
	virtual	~CameraI();
	std::string	getName(const Ice::Current& current);
	int	nCcds(const Ice::Current& current);
	CcdInfo	getCcdinfo(int ccdid, const Ice::Current& current);
	CcdPrx	getCcd(int ccdid, const Ice::Current& current);
	bool	hasFilterWheel(const Ice::Current& current);
	FilterWheelPrx	getFilterWheel(const Ice::Current& current);
	bool	hasGuiderPort(const Ice::Current& current);
	GuiderPortPrx	getGuiderPort(const Ice::Current& current);
static	CameraPrx	createProxy(const std::string& cameraname,
				const Ice::Current& current);
};

} // namespace snowstar

#endif /* _CameraI_h */
