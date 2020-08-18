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
	// XXX we need a monitoring thread for the cooler
	std::thread			_thread;
	std::recursive_mutex		_mutex;
	std::condition_variable_any	_condition;
	bool	_running;
	static void	main(AtikCooler *cooler) noexcept;
	void	run();
	void	stop();
	friend class AtikCamera;
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
