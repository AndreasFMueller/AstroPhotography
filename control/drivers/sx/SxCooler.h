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
	void	cmd();
	void	query(bool sendcallback = false);
	// separate thread for cooler monitoring
	std::thread		_thread;
	std::recursive_mutex	_mutex;
	std::condition_variable_any	_cond;
	bool			_terminate;
public:
	SxCooler(SxCamera& camera);
	virtual	~SxCooler();
	//virtual float	getSetTemperature();
	virtual Temperature	getActualTemperature();
	virtual void	setTemperature(float temperature);
	virtual bool	isOn();
	virtual	void	setOn(bool onoff);
	void	run();
};

} // namespace sx
} // namespace camera
} // namespace astro

#endif /* _SxCooler_h */
