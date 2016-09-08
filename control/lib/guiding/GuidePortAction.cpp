/*
 * GuidePortAction.cpp -- implementation of guider port action
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include "GuidePortAction.h"

using namespace astro::callback;

namespace astro {
namespace guiding {

/**
 * \brief execute the guideport action
 */
void	GuidePortAction::execute() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guider port action started %s",
		_correction.toString().c_str());

	if (!((_correction.x() == _correction.x()) &&
		(_correction.y() == _correction.y()))) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "nan correction, giving up");
		return;
	}

	double	tx = 0;
	double	ty = 0;

	if (fabs(_correction.x()) > _deltat) {
		tx = (_correction.x() > 0) ? _deltat : -_deltat;
	} else {
		tx = _correction.x();
	}
	if (fabs(_correction.y()) > _deltat) {
		ty = (_correction.y() > 0) ? _deltat : -_deltat;
	} else {
		ty = _correction.y();
	}

	// make sure the time fits into the allotted time
	double	limit = 0;
	if (_sequential) {
		limit = fabs(tx) + fabs(ty);
	} else {
		limit = std::max(fabs(tx), fabs(ty));
	}
	if (limit > _deltat) {
		tx *= _deltat / limit;
		ty *= _deltat / limit;
		limit = _deltat;
	}

	// compute the activation times for the guideport
	double	raplus = 0;
	double	raminus = 0;
	double	decplus = 0;
	double	decminus = 0;
	double	correctiontime = std::max(fabs(tx), fabs(ty));
	
	if (tx > 0) {
		raplus = tx;
	} else {
		raminus = -tx;
	}
	if (ty > 0) {
		decplus = ty;
	} else {
		decminus = -ty;
	}

	if (_sequential) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "RA movement: %.2f", tx);
		_guideport->activate(raplus, raminus, 0, 0);
		Timer::sleep(fabs(tx));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "DEC movement: %.2f", ty);
		_guideport->activate(0, 0, decplus, decminus);
		Timer::sleep(fabs(ty));
	} else {
		// find the number of seconds, and split the correction
		// into this many single steps. This makes the 
		int	steps = 1;
		if (_stepping) {
			steps = floor(_deltat);
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "RA/DEC %.2f/%.2f in %d steps",
			tx, ty, steps);
		raplus /= steps;
		raminus /= steps;
		decplus /= steps;
		decminus /= steps;
		int	step = 0;
		while (step++ < steps) {
			_guideport->activate(raplus, raminus,
					decplus, decminus);
			Timer::sleep(correctiontime);
		}
	}
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guider port action complete");
}

} // namespace guiding
} // namespace astro
