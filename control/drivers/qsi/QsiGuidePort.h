/*
 * QsiGuidePort.h -- QSI camera guider port
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _QsiGuidePort_h
#define _QsiGuidePort_h

#include <AstroCamera.h>
#include <QsiCamera.h>

namespace astro {
namespace camera {
namespace qsi {

class QsiGuidePort : public GuidePort {
	QsiCamera&	_camera;
public:
	QsiGuidePort(QsiCamera& camera);
	virtual ~QsiGuidePort();
	virtual uint8_t	active();
	virtual void	activate(float raplus, float raminus,
				float decplus, float decminus);
};

} // namespace qsi
} // namespace camera
} // namespace astro

#endif /* _QsiGuidePort_h */
