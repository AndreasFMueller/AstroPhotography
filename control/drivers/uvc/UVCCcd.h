/*
 * UvcCcd.h -- ccd implementation for the UVC driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <UvcCamera.h>

using namespace astro::image;

namespace astro {
namespace camera {
namespace uvc {

class UvcCcd : public Ccd {
	int	interface;
	int	format;
	int	frame;
	UvcCamera&	camera;
public:
	UvcCcd(const CcdInfo& info, int interface, int format, int frame,
		UvcCamera& camera);
	virtual void    startExposure(const Exposure& exposure)
		throw (not_implemented);
	virtual ImagePtr	getImage() throw(not_implemented);
	virtual ImageSequence	getImageSequence(unsigned int imagecount)
		throw(not_implemented);
};

} // namespace uvc
} // namespace camera
} // namespace astro
