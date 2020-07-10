/*
 * SbigCooler.h -- TE cooler for SBIG cameras
 * 
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SbigCooler_h
#define _SbigCooler_h

#include <SbigCamera.h>
#include <SbigDevice.h>

using namespace astro::camera;

namespace astro {
namespace camera {
namespace sbig {

class SbigCooler : public Cooler, public SbigDevice {
	void	set();
	void	query_temperature_status(
			QueryTemperatureStatusResults2  *results);
	void	set_temperature_regulation2(
			SetTemperatureRegulationParams2 *params);
public:
	SbigCooler(SbigCamera& camera, const DeviceName& devname);
	~SbigCooler();
	virtual Temperature	getSetTemperature();
	virtual Temperature	getActualTemperature();
	virtual void	setTemperature(const float temperature);
	virtual bool	isOn();
	virtual void	setOn(bool onoff);
};

} // namespace sbig
} // namespace camera
} // namespace astro

#endif /* _SbigCooler_h */
