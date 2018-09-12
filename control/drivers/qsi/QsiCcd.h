/*
 * QsiCcd.h -- CCD interface for QSI cameras
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _QsiCcd_h
#define _QsiCcd_h

#include <AstroCamera.h>
#include <QsiCamera.h>

namespace astro {
namespace camera {
namespace qsi {

class QsiCcd : public Ccd {
	QsiCamera&	_camera;
public:
	QsiCcd(const CcdInfo&, QsiCamera& camera);
	virtual ~QsiCcd();
	virtual void	startExposure(const Exposure& exposure);
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
};

} // namespace qsi
} // namespace camera
} // namespace astro


#endif /* _QsiCcd_h */
