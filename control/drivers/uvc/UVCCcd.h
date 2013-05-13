/*
 * UVCCcd.h -- ccd implementation for the UVC driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <UVCCamera.h>

namespace astro {
namespace camera {
namespace uvc {

class UVCCcd : public Ccd {
	int	interface;
	int	format;
	int	frame;
	UVCCamera&	camera;
public:
	UVCCcd(const CcdInfo& info, int interface, int format, int frame,
		UVCCamera& camera);
	virtual void    startExposure(const Exposure& exposure)
		throw (not_implemented);
	virtual ImagePtr	getImage() throw(not_implemented);
};

} // namespace uvc
} // namespace camera
} // namespace astro
