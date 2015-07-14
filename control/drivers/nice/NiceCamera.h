/*
 * NiceCamera.h -- ICE camera wrapper
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _NiceCamera_h
#define _NiceCamera_h

#include <AstroCamera.h>
#include <camera.h>
#include <NiceDevice.h>

using namespace astro;

namespace astro {
namespace camera {
namespace nice {

/**
 * \brief Wrapper for cameras to be accessed via ICE
 */
class NiceCamera : public astro::camera::Camera, public NiceDevice {
	snowstar::CameraPrx	_camera;
public:
	NiceCamera(snowstar::CameraPrx camera, const DeviceName& devicename);
	virtual ~NiceCamera();

protected:
	virtual astro::camera::CcdPtr	getCcd0(size_t ccdid);

public:
	virtual bool	hasGuiderPort() const;
protected:
	virtual astro::camera::GuiderPortPtr	getGuiderPort0();

public:
	virtual bool	hasFilterWheel() const;
protected:
	virtual astro::camera::FilterWheelPtr	getFilterWheel0();
};

} // namespace nice
} // namespace camera
} // namespace astro

#endif /* _NiceCamera_h */
