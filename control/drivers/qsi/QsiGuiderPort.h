/*
 * QsiGuiderPort.h -- QSI camera guider port
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _QsiGuiderPort_h
#define _QsiGuiderPort_h

#include <AstroCamera.h>
#include <QsiCamera.h>

namespace astro {
namespace camera {
namespace qsi {

class QsiGuiderPort : public GuiderPort {
	QsiCamera&	_camera;
public:
	QsiGuiderPort(QsiCamera& camera);
	virtual ~QsiGuiderPort();
	virtual uint8_t	active();
	virtual void	activate(float raplus, float raminus,
				float decplus, float decminus);
};

} // namespace qsi
} // namespace camera
} // namespace astro

#endif /* _QsiGuiderPort_h */
