/*
 * QsiCcd.h -- CCD interface for QSI cameras
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _QsiCcd_h
#define _QsiCcd_h

#include <AstroCamera.h>
#include <QsiCamera.h>
#include <atomic>

namespace astro {
namespace camera {
namespace qsi {

class QsiCcd : public Ccd {
	QsiCamera&	_camera;
	// thread waiting for the exposure to complete
	std::thread		_thread;
public:
	QsiCcd(const CcdInfo&, QsiCamera& camera);
	virtual ~QsiCcd();
	virtual void	startExposure(const Exposure& exposure);
private:
	// manage the thread
	static void	start_main(QsiCcd *qsiccd) noexcept;
	void	run();
	void	wait_thread();
	void	stop();
public:
	virtual CcdState::State	exposureStatus();
	virtual void	cancelExposure();

	// shutter stuff
	virtual Shutter::state	getShutterState();
	virtual void	setShutterState(const Shutter::state& state);

	// image retrieval
	virtual astro::image::ImagePtr	getRawImage();

protected:
	CoolerPtr	_cooler;
	virtual CoolerPtr	getCooler0();
public:
	virtual bool	hasCooler() const { return true; }

	virtual std::string	userFriendlyName() const {
		return _camera.userFriendlyName();
	}
private:
	bool	_cansetgain;
public:
	virtual bool	hasGain() { return _cansetgain; }
	virtual float	getGain();
	virtual std::pair<float, float>	gainInterval();

	friend class QsiCamera;
};

} // namespace qsi
} // namespace camera
} // namespace astro

#endif /* _QsiCcd_h */
