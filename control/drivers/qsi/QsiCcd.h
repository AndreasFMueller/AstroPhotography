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
	std::atomic<CcdState::State>	_last_state;
	// thread waiting for the exposure to complete
	std::thread		*_thread;
	std::atomic_bool	_exposure_done;
public:
	QsiCcd(const CcdInfo&, QsiCamera& camera);
	virtual ~QsiCcd();
	virtual void	startExposure(const Exposure& exposure);
	void	run();
	virtual CcdState::State	exposureStatus();
	virtual void	cancelExposure();

	// shutter stuff
	virtual Shutter::state	getShutterState();
	virtual void	setShutterState(const Shutter::state& state);

	// image retrieval
	virtual astro::image::ImagePtr	getRawImage();

protected:
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
};

} // namespace qsi
} // namespace camera
} // namespace astro

#endif /* _QsiCcd_h */
