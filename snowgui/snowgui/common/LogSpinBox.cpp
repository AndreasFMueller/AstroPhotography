/*
 * LogSpinBox.cpp -- logarithmic spin box used for expsure times
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <logspinbox.h>
#include <math.h>
#include <AstroDebug.h>

namespace snowgui {

/**
 * \brief Constructor for LogSpinBox
 */
LogSpinBox::LogSpinBox(QWidget *parent) : QDoubleSpinBox(parent) {
}

/**
 * \brief Compute step size for upward step
 *
 * Upward steps essentially double the exposure time for small exposure times,
 * and are one minute steps when the exposure time is already at least
 * one minute
 */
double	LogSpinBox::upstep() {
	double	currentstep = value();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "upstep(%.4f)", currentstep);
	// if we get to the next minute by double, we change the step size
	// in such a way that the next up step gets us exactly to one minute
	// this happens when the step is between 30 and 60
	if ((30 <= currentstep) && (currentstep < 60)) {
		currentstep = 60 - value();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "step to next minute: %.4f",
			currentstep);
		return currentstep;
	}
	// if the current step size is larger that 60 seconds, we reduce it
	// to one minute
	if (currentstep > 60) {
		currentstep = 60;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new upstep: %.4f", currentstep);
	return currentstep;
}

/**
 * \brief Compute step size for downward steps
 *
 * Downward steps half the exposure time for small exposure times,
 * and subtract to the next minute when the exposure time is larger
 * than one minute
 */
double	LogSpinBox::downstep() {
	double	currentstep = value();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "downstep(%.4f)", currentstep);
	// if the current exposure time is more than one minute, then the
	// next step should bring us down to the next minute
	if (value() > 60) {
		currentstep -= 60 * trunc(currentstep / 60);
		if (currentstep == 0) {
			currentstep = 60;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "step to next minute: %.4f",
			currentstep);
		return currentstep;
	}
	// in all other cases, we want to half the current value
	currentstep /= 2;
	double	multiplier = 1;
	for (int i = 0; i < decimals(); i++) {
		multiplier *= 10;
	}
	currentstep = round(currentstep * multiplier) / multiplier;
	if (currentstep < minimum()) {
		currentstep = minimum();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new downstep: %.4f", currentstep);
	return currentstep;
}

/**
 * \brief Reimplementation of the QWidghet stepBy method
 */
void	LogSpinBox::stepBy(int steps) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "steps: %d", steps);
	if (steps > 0) {
		// if stepping up, then we double the step size from the
		// current vlaue, but never increase beyond 60s
		for (int step = 0; step < steps; step++) {
			setSingleStep(upstep());
		}
	}
	if (steps < 0) {
		// if stepping down, we half the step size in each step,
		// but rounding to the current precision
		for (int step = 0; step > steps; step--) {
			setSingleStep(downstep());
		}
	}
	QDoubleSpinBox::stepBy(steps);
}

} // namespace snowgui
