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
	std::mutex	_mutex;
	std::condition_variable	_condition;
	std::thread	_thread;
	bool		_running;
	void	setCoolerTemperature();
public:
	AsiCooler(AsiCamera& camera);
	~AsiCooler();
	virtual Temperature	getSetTemperature();
	virtual Temperature	getActualTemperature();
protected:
	virtual void	setTemperature(const float temperature);
public:
	virtual bool	isOn();
	virtual void	setOn(bool onoff);
	// dew heater stuff
	virtual bool	hasDewHeater();
	virtual float	dewHeater();
	virtual void	dewHeater(float dewheatervalue);
	virtual std::pair<float, float>	dewHeaterRange();
private:
	static void	main(AsiCooler *cooler) noexcept;
	void	run();
	void	stop();
	friend class AsiCamera;
};

} // namespace asi
} // namespace camera
} // namespace astro

#endif /* _AsiCooler_h */
