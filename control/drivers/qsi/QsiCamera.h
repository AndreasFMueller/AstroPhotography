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

class QsiCamera : public Camera {
	QSICamera	camera;
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
