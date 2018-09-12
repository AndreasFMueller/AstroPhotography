/*
 * AtikCooler.h -- Atik Cooler class
 *
 * (c) 2016 Prof Dr Andreas Müller
 */
#ifndef _AtikCooler_h
#define _AtikCooler_h

#include <atikccdusb.h>
#include <AtikCamera.h>

namespace astro {
namespace camera {
namespace atik {

class AtikCooler : public Cooler {
	AtikCamera&	_camera;
	bool	_lastIsOn;
	float	_lastTemperature;
	float	_lastSetTemperature;
public:
	AtikCooler(AtikCamera&);
	virtual ~AtikCooler();
	virtual float	getSetTemperature();
	virtual float	getActualTemperature();
	virtual void	setTemperature(const float temperature);
	virtual bool	isOn();
	virtual void	setOn(bool onoff);
	virtual std::string	userFriendlyName() const {
		return _camera.userFriendlyName();
	}
};

} // namespace atik
} // namespace camera 
} // namespace astro

#endif /* _AtikCooler_h */
