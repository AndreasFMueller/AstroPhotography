/*
 * AtikCamera.h -- Atik camera class
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _AtikCamera_h
#define _AtikCamera_h

#include <atikccdusb.h>
#include <AstroCamera.h>

namespace astro {
namespace camera {
namespace atik {

class AtikCamera : public Camera {
	::AtikCamera	*_camera;
	struct AtikCapabilities	capa;
public:
	AtikCamera(::AtikCamera *camera);
	virtual ~AtikCamera();
	CcdPtr	getCcd0(size_t ccdid);
	unsigned int	nCcds() const;

protected:
	virtual FilterWheelPtr	getFilterWheel0();
public:
	bool	hasFilterWheel() const;

protected:
	virtual GuidePortPtr	getGuidePort0();
public:
	bool	hasGuidePort() const;

friend class AtikCcd;
};

} // namespace atik
} // namespace camera
} // namespace astro

#endif /* _AtikCamera_h */
