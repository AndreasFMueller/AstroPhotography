/*
 * AsiCooler.h -- Cooler for ASI cooled camera declaration
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _AsiCooler_h
#define _AsiCooler_h

#include <AstroCamera.h>
#include <AsiCamera.hh>
#include <AsiCcd.h>

namespace astro {
namespace camera {
namespace asi {

/**
 * \brief Implementation class for the Cooler on an ASI cooled camera
 */
class AsiCooler : public Cooler {
	AsiCamera&	_camera;
	void	setCoolerTemperature();
public:
	AsiCooler(AsiCamera& camera, AsiCcd& ccd);
	~AsiCooler();
	virtual float	getSetTemperature();
	virtual float	getActualTemperature();
	virtual void	setTemperature(const float temperature);
	virtual bool	isOn();
	virtual void	setOn(bool onoff);
	// dew heater stuff
	virtual bool	hasDewHeater();
	virtual float	dewHeater();
	virtual void	dewHeater(float dewheatervalue);
	virtual std::pair<float, float>	dewHeaterRange();
};

} // namespace asi
} // namespace camera
} // namespace astro

#endif /* _AsiCooler_h */
