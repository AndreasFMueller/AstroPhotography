/*
 * Qhy2Cooler.h -- cooler class
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Qhy2Cooler_h
#define _Qhy2Cooler_h

#include <Qhy2Camera.h>
#include <qhyccd.h>

namespace astro {
namespace camera {
namespace qhy2 {

/**
 * \brief QHY cooler class
 *
 * This class has no state on it's own, it just uses the state available
 * in the deviceptr member.
 */
class Qhy2Cooler : public Cooler {
public:
	Qhy2Cooler(Qhy2Camera& _camera);
	virtual ~Qhy2Cooler();
	virtual Temperature	getActualTemperature();
	virtual void	setTemperature(float temperature);
	virtual bool	isOn();
	virtual void	setOn(bool onoff);
};

} // namespace qhy2
} // namespace camera
} // namespace astro

#endif /* _Qhy2Cooler_h */
