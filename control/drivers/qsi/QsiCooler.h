/*
 * QsiCooler.h -- QSI cooler declarations
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _QsiCooler_h
#define _QsiCooler_h

#include <AstroCamera.h>
#include <QsiCamera.h>
#include <atomic>

namespace astro {
namespace camera {
namespace qsi {

class QsiCooler : public Cooler {
	QsiCamera&	_camera;
	// thread for cooler monitoring
	std::thread			_thread;
	std::recursive_mutex		_mutex;
	std::condition_variable_any	_condition;
	bool				_running;
	static void	start_main(QsiCooler* cooler) noexcept;
	void	run();
public:
	QsiCooler(QsiCamera& camera);
	virtual ~QsiCooler();
	virtual Temperature	getSetTemperature();
	virtual Temperature	getActualTemperature();
	virtual void	setTemperature(const float temperature);
	virtual bool	isOn();
	virtual void	setOn(bool onoff);
	virtual std::string	userFriendlyName() const {
		return _camera.userFriendlyName();
	}
	void	stop();
};

} // namespace qsi
} // namespace camera
} // namespace astro

#endif /* _QsiCooler_h */
