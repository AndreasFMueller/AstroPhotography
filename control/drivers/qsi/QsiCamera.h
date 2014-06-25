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

namespace astro {
namespace camera {
namespace qsi {

class QsiCcd;
class QsiCooler;
class QsiFilterWheel;
class QsiGuiderPort;

class QsiCamera : public Camera {
	QSICamera	_camera;
	QSICamera&	camera() { return _camera; }
friend class QsiCcd; // allow the CCD to get the QSICamera
friend class QsiCooler; // allow the Cooler to get the QSICamera
friend class QsiFilterWheel; // allow the Filterwheel to get the QSICamera
friend class QsiGuiderPort; // allow the GuiderPort to get the QSICamera
	bool	_hasfilterwheel;
	bool	_hasguiderport;
public:
	QsiCamera(const std::string& name);
	virtual ~QsiCamera();

	virtual void	reset();

protected:
	virtual CcdPtr	getCcd0(size_t id);

public:
	virtual bool	hasFilterWheel() const;
protected:
	virtual FilterWheelPtr	getFilterWheel0();

public:
	virtual bool	hasGuiderPort() const;
protected:
	virtual GuiderPortPtr	getGuiderPort0();

public:
	bool	isColor() const;
};

} // namespace qsi
} // namespace camera
} // namespace astro

#endif /* _QsiCamera_h */
