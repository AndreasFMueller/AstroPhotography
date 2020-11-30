/*
 * QhyCooler.h -- cooler class
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _QhyCooler_h
#define _QhyCooler_h

#include <QhyCamera.h>
#include <qhylib0.h>

namespace astro {
namespace camera {
namespace qhy {

/**
 * \brief QHY cooler class
 *
 * This class has no state on it's own, it just uses the state available
 * in the deviceptr member.
 */
class QhyCooler : public Cooler {
	::qhy::DevicePtr	deviceptr;
public:
	QhyCooler(QhyCamera& _camera, ::qhy::DevicePtr devptr);
	virtual ~QhyCooler();
	virtual Temperature	getActualTemperature();
	virtual void	setTemperature(float temperature);
	virtual bool	isOn();
	virtual void	setOn(bool onoff);
};

} // namespace qhy
} // namespace camera
} // namespace astro

#endif /* _QhyCooler_h */
