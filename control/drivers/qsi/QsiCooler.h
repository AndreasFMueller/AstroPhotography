/*
 * QsiCooler.h -- QSI cooler declarations
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _QsiCooler_h
#define _QsiCooler_h

#include <AstroCamera.h>
#include <QsiCamera.h>

namespace astro {
namespace camera {
namespace qsi {

class QsiCooler : public Cooler {
	QsiCamera&	_camera;
	float	_actual_temperature;
	bool	_is_on;
public:
	QsiCooler(QsiCamera& camera);
	virtual float	getSetTemperature();
	virtual float	getActualTemperature();
	virtual void	setTemperature(const float temperature);
	virtual bool	isOn();
	virtual void	setOn(bool onoff);
	virtual std::string	userFriendlyName() const {
		return _camera.userFriendlyName();
	}
};

} // namespace qsi
} // namespace camera
} // namespace astro

#endif /* _QsiCooler_h */
