/*
 * SbigCooler.h -- TE cooler for SBIG cameras
 * 
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SbigCooler_h
#define _SbigCooler_h

#include <SbigCamera.h>

using namespace astro::camera;

namespace astro {
namespace camera {
namespace sbig {

class SbigCooler : public Cooler {
	SbigCamera&	camera;
	bool	enabled;
	void	set();
public:
	SbigCooler(SbigCamera& camera);
	~SbigCooler();
	virtual float	getSetTemperature();
	virtual float	getActualTemperature();
	virtual void	setTemperature(const float temperature);
	virtual bool	isOn();
	virtual void	setOn(bool onoff);
};

} // namespace sbig
} // namespace camera
} // namespace astro

#endif /* _SbigCooler_h */
