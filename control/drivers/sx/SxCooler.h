/*
 * SxCooler.h -- abstraction for the CCD of a starlight express camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SxCooler_h
#define _SxCooler_h

#include "SxCamera.h"

using namespace astro::camera;
using namespace astro::image;

namespace astro {
namespace camera {
namespace sx {

/**
 * \brief Starlight Express Cooler abstraction
 *
 * The Starlight Express cameras do have a cooler and a proprietary API,
 * this class encapsulates that.
 */
class SxCooler : public Cooler {
	SxCamera&	camera;
	bool	cooler_on;
	float	settemperature;
	float	actualtemperature;
	void	cmd();
public:
	SxCooler(SxCamera& camera);
	virtual	~SxCooler();
	//virtual float	getSetTemperature();
	virtual float	getActualTemperature();
	virtual void	setTemperature(float temperature);
	virtual bool	isOn();
	virtual	void	setOn(bool onoff);
};

} // namespace sx
} // namespace camera
} // namespace astro

#endif /* _SxCooler_h */
