/*
 * GuidePortActivation.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rappeswil
 */
#include <AstroCamera.h>

namespace astro {
namespace camera {

GuidePortActivation::GuidePortActivation() {
	_raplus = 0.;
	_raminus = 0.;
	_decplus = 0.;
	_decminus = 0.;
}

GuidePortActivation::GuidePortActivation(GuidePortActivation::direction_t dir,
	float time) {
	_raplus = 0.;
	_raminus = 0.;
	_decplus = 0.;
	_decminus = 0.;
	switch (dir) {
	case RAPLUS:	_raplus = time;		break;
	case RAMINUS:	_raminus = time;	break;
	case DECPLUS:	_decplus = time;	break;
	case DECMINUS:	_decminus = time;	break;
	}
}

GuidePortActivation::GuidePortActivation(float raplus, float raminus,
	float decplus, float decminus) : _raplus(raplus), _raminus(raminus),
	_decplus(decplus), _decminus(decminus) {
}

float	GuidePortActivation::max() const {
	return std::max(std::max(_raplus, _raminus), std::max(_decplus, _decminus));
}

} // namespace camea
} // namespace astro
