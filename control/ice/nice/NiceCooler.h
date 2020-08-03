/*
 * NiceCooler.h -- wrapper fro coolers
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _NiceCooler_h
#define _NiceCooler_h

#include <AstroCamera.h>
#include <camera.h>
#include <NiceDevice.h>

namespace astro {
namespace camera {
namespace nice {

class NiceCooler;
class NiceCoolerCallbackI : public snowstar::CoolerCallback {
	NiceCooler&	_cooler;
public:
	NiceCoolerCallbackI(NiceCooler& cooler) : _cooler(cooler) { }
	virtual void	updateCoolerInfo(const snowstar::CoolerInfo& info,
				const Ice::Current& current);
	virtual void	updateSetTemperature(Ice::Float settemperature,
				const Ice::Current& current);
	virtual void	updateDewHeater(Ice::Float dewheater,
				const Ice::Current& current);
	virtual void	stop(const Ice::Current& current);
};

class NiceCooler : public Cooler, public NiceDevice {
	snowstar::CoolerPrx	_cooler;
	Ice::ObjectPtr	_cooler_callback;
	Ice::Identity	_cooler_identity;
public:
	NiceCooler(snowstar::CoolerPrx cooler, const DeviceName& devicename);
	virtual ~NiceCooler();

	virtual Temperature getSetTemperature();
	virtual Temperature getActualTemperature();
protected:
	virtual void	setTemperature(float temperature);
public:
	virtual bool	isOn();
	virtual void	setOn(bool onoff);

	virtual bool	hasDewHeater();
	virtual float	dewHeater();
	virtual void	dewHeater(float dewheatervalue);
	virtual std::pair<float, float>	dewHeaterRange();
};

} // namespace nice
} // namespace camera
} // namespace astro

#endif /* _NiceCooler_h */
