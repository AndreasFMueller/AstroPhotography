/*
 * SbigCcd.h -- Sbig camera ccd
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SbigCcd_h
#define _SbigCcd_h

#include <AstroCamera.h>
#include <AstroImage.h>
#include <SbigCamera.h>

using namespace astro::camera;
using namespace astro::image;

namespace astro {
namespace camera {
namespace sbig {

class SbigCcd : public Ccd {
	int	id;
	SbigCamera&	camera;
	Exposure	exposure;
public:
	SbigCcd(const ImageSize& size, int id, SbigCamera& camera);
	virtual ~SbigCcd();
	virtual Exposure::State	exposureStatus() throw (not_implemented);
	virtual void	startExposure(const Exposure& exposure) throw (not_implemented);
	virtual	ShortImagePtr	shortImage() throw(not_implemented);
};

} // namespace sbig
} // namespace camera
} // namespace astro

#endif /* _SbigCcd_h */
