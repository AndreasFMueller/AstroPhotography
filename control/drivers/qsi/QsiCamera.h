/*
 * QsiCamera.h -- QSI camera abstraction
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _QsiCamera_h
#define _QsiCamera_h

#include <AstroCamera.h>
#include <qsiapi.h>
#include <QSIError.h>
#include <atomic>

namespace astro {
namespace camera {
namespace qsi {

class QsiCcd;
class QsiCooler;
class QsiFilterWheel;
class QsiGuidePort;

class QsiCamera : public Camera {
	std::recursive_mutex	mutex;
	QSICamera	_camera;
	QSICamera&	camera() { return _camera; }
friend class QsiCcd; // allow the CCD to get the QSICamera
friend class QsiCooler; // allow the Cooler to get the QSICamera
friend class QsiFilterWheel; // allow the Filterwheel to get the QSICamera
friend class QsiGuidePort; // allow the GuidePort to get the QSICamera
	bool	_hasfilterwheel;
	bool	_hasguideport;
public:
	QsiCamera(const std::string& name);
	virtual ~QsiCamera();

	virtual void	reset();

private:
	CcdPtr	_ccd;
protected:
	virtual CcdPtr	getCcd0(size_t id);

public:
	virtual bool	hasFilterWheel() const;
private:
	FilterWheelPtr	_filterwheel;
protected:
	virtual FilterWheelPtr	getFilterWheel0();

public:
	virtual bool	hasGuidePort() const;
protected:
	virtual GuidePortPtr	getGuidePort0();

public:
	bool	isColor() const;

private:
	std::string	_userFriendlyName;
public:
	virtual std::string	userFriendlyName() const {
		return _userFriendlyName;
	}
};

} // namespace qsi
} // namespace camera
} // namespace astro

#endif /* _QsiCamera_h */
