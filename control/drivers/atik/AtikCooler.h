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
	Temperature	_lastTemperature;
	Temperature	_lastSetTemperature;
public:
	AtikCooler(AtikCamera&);
	virtual ~AtikCooler();
	virtual Temperature	getSetTemperature();
	virtual Temperature	getActualTemperature();
protected:
	virtual void	setTemperature(const float temperature);
public:
	virtual bool	isOn();
	virtual void	setOn(bool onoff);
	virtual std::string	userFriendlyName() const {
		return _camera.userFriendlyName();
	}
	void	overrideSetTemperature(float temperature);
};

} // namespace atik
} // namespace camera 
} // namespace astro

#endif /* _AtikCooler_h */
