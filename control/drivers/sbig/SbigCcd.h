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
	bool	cooler;
public:
	SbigCcd(const CcdInfo& info, int id, SbigCamera& camera);
	virtual ~SbigCcd();
	virtual Exposure::State	exposureStatus();
	virtual void	startExposure(const Exposure& exposure);

	// shutter interface
	virtual Shutter::state	getShutterState();
	virtual void	setShutterState(const Shutter::state& state);

	virtual	ImagePtr	getRawImage();

	// cooler interface
	virtual bool	hasCooler() const { return cooler; }
	void	setCooler(bool _cooler) { cooler = _cooler; }
protected:
	virtual CoolerPtr	getCooler0();
};

} // namespace sbig
} // namespace camera
} // namespace astro

#endif /* _SbigCcd_h */
