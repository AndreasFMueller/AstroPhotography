/*
 * GridCalculator.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil 
 */
#include <AstroCoordinates.h>
#include <AstroDebug.h>

namespace astro {
namespace utils {

/**
 * \brief Construct the Grid calculator
 *
 * \param center		The RA and DEC of the FOV center
 * \param frame			The pixel dimensions of the FOV
 * \param pixels_per_degree	The pixel resolution
 */
GridCalculator::GridCalculator(const RaDec& center, const Size& frame,
	double pixels_per_degree)
	: _center(center), _frame(frame), _pixels_per_degree(pixels_per_degree) {
	_minra = 0; _maxra = 0;
	_mindec = 0; _maxdec = 0;
}

static Angle	round_to_degrees(Angle dec_angle) {
	Angle	degreeangles[14] = {
		Angle(30, Angle::Degrees),
		Angle(20, Angle::Degrees),
		Angle(15, Angle::Degrees),
		Angle(10, Angle::Degrees),
		Angle(5, Angle::Degrees),
		Angle(2, Angle::Degrees),
		Angle(1, Angle::Degrees),
		Angle(30, Angle::ArcMinutes),
		Angle(20, Angle::ArcMinutes),
		Angle(15, Angle::ArcMinutes),
		Angle(10, Angle::ArcMinutes),
		Angle(5, Angle::ArcMinutes),
		Angle(2, Angle::ArcMinutes),
		Angle(1, Angle::ArcMinutes)
	};
	double	d[14];
	double	minvalue = 360;
	int	minindex = -1;
	for (int i = 0; i < 14; i++) {
		d[i] = fabs(degreeangles[i].degrees() - dec_angle.degrees());
		if (d[i] < minvalue) {
			minvalue = d[i];
			minindex = i;
		}
	}
	return degreeangles[minindex];
}

static Angle	round_to_hours(Angle ra_angle) {
	Angle	hourangles[17] = {
		Angle(3, Angle::Hours),
		Angle(2, Angle::Hours),
		Angle(1, Angle::Hours),
		Angle(30, Angle::Minutes),
		Angle(20, Angle::Minutes),
		Angle(15, Angle::Minutes),
		Angle(10, Angle::Minutes),
		Angle(5, Angle::Minutes),
		Angle(2, Angle::Minutes),
		Angle(1, Angle::Minutes),
		Angle(30, Angle::Seconds),
		Angle(20, Angle::Seconds),
		Angle(15, Angle::Seconds),
		Angle(10, Angle::Seconds),
		Angle(5, Angle::Seconds),
		Angle(2, Angle::Seconds),
		Angle(1, Angle::Seconds)
	};
	double	d[17];
	double	minvalue = 360;
	int	minindex = -1;
	for (int i = 0; i < 14; i++) {
		d[i] = fabs(hourangles[i].degrees() - ra_angle.degrees());
		if (d[i] < minvalue) {
			minvalue = d[i];
			minindex = i;
		}
	}
	return hourangles[minindex];
}

/**
 * \brief Compute the grid parameters
 *
 * This method computes the grid step sizes so that in the center of the image,
 * RA and DEC grid lines are spaced equally. It also computes the rounded center of
 * the grid and the ranges of multiples of grid steps that need to be covered for
 * a complete grid.
 *
 * \param pixelstep
 */
void	GridCalculator::gridsetup(double pixelstep) {
	// get the ra step size
	double	dec_angle = pixelstep / _pixels_per_degree;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "dec_angle = %f", dec_angle);
	Angle	dec = round_to_degrees(Angle(dec_angle, Angle::Degrees));

	// get the dec step size
	double	ra_angle = pixelstep / (_pixels_per_degree * cos(_center.dec()));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ra_angle = %f", (12. / 180.) * dec_angle);
	Angle	ra = round_to_hours(Angle(ra_angle, Angle::Degrees));

	// set the step sizes
	_stepsizes = RaDec(ra, dec);

	// round the center angle to the nearest multiple of the step size
	Angle	cra = round(_center.ra() / _stepsizes.ra()) * _stepsizes.ra();
	Angle	cdec = round(_center.dec() / _stepsizes.dec()) * _stepsizes.dec();
	_gridzero = RaDec(cra, cdec);

	// angles needed for the triangle computations
	Angle	a = Angle::right_angle - _center.dec();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "a = %s", a.dms().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "w = %f, h = %f",
		_frame.width(), _frame.height());
	Angle	beta0 = astro::arctan2(_frame.width(), _frame.height());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "beta0 = %s", beta0.dms().c_str());

	// compute the angle c
	double	pixels_per_radian = 180 * _pixels_per_degree / M_PI;
	Angle	c = astro::arctan((_frame.diagonal() / 2) / pixels_per_radian);

	// compute upper right corner
	double	cosb_ur = cos(a) * cos(c) + sin(a) * sin(c) * cos(beta0);
	Angle	b_ur = astro::arccos(cosb_ur);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "b = %s", b_ur.dms().c_str());
	double	singamma_ur = sin(c) * sin(beta0) / sin(b_ur);
	double	cosgamma_ur = (cos(c) - cos(a) * cos(b_ur)) / (sin(a) * sin(b_ur));
	Angle	gamma_ur = abs(astro::arctan2(singamma_ur, cosgamma_ur));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "gamma = %s", gamma_ur.dms().c_str());

	// compute lower right corner
	double	cosb_lr = cos(a) * cos(c) - sin(a) * sin(c) * cos(beta0);
	Angle	b_lr = astro::arccos(cosb_lr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "b = %s", b_lr.dms().c_str());
	double	singamma_lr = sin(c) * sin(beta0) / sin(b_lr);
	double	cosgamma_lr = (cos(c) - cos(a) * cos(b_lr)) / (sin(a) * sin(b_lr));
	Angle	gamma_lr = abs(astro::arctan2(singamma_lr, cosgamma_lr));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "gamma = %s", gamma_lr.dms().c_str());

	// determine the bigger of the two angles gamma_*, this fixes the range
	// in RA
	Angle	gamma = gamma_lr;
	if (gamma_ur > gamma_lr) {
		gamma = gamma_ur;
	}

	// compute the maximum and minimu for ra
	Angle	delta_ra = _gridzero.ra() - _center.ra();
	_maxra = trunc((gamma - delta_ra) / _stepsizes.ra()) + 1;
	_minra = trunc((-gamma - delta_ra) / _stepsizes.ra()) - 1;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "minra = %d, maxra = %d", _minra, _maxra);

	// the angular height of the image 
	Angle	epsilon = astro::arctan((_frame.height() / 2) / pixels_per_radian);

	// find out whether the pole is inside the image
	_pole_in_frame = (_center.dec() + epsilon > Angle::right_angle) ||
			(_center.dec() - epsilon < -Angle::right_angle);

	// absolute max/min dec
	Angle	topdec = Angle::right_angle - _gridzero.dec();
	int	dec_abs_max = round(topdec / _stepsizes.dec()) - 1;
	Angle	bottomdec = -Angle::right_angle - _gridzero.dec();
	int	dec_abs_min = round(bottomdec / _stepsizes.dec()) + 1;

	// compute maximum dec
	Angle	topcenter = _center.dec() + epsilon;
	if (topcenter > Angle(0)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "top border %s above equator, "
			"center determines maximum", topcenter.dms().c_str());
		_maxdec = round((topcenter - _gridzero.dec())
			/ _stepsizes.dec());
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "top border %s below equator, "
			"corner determines maximum", topcenter.dms().c_str());
		Angle	g = Angle::right_angle - b_ur;
		_maxdec = round((g - _gridzero.dec()) / _stepsizes.dec());
	}
	_maxdec += 1;
	if (_maxdec > dec_abs_max) {
		_maxdec = dec_abs_max;
	}

	// compute minimum dec
	Angle	bottomcenter = _center.dec() - epsilon;
	if (bottomcenter < Angle(0)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "bottom border %s below equator, "
			"center determines minimum", bottomcenter.dms().c_str());
		_mindec = round((bottomcenter - _gridzero.dec())
			/ _stepsizes.dec());
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "bottom border %s above equator, "
			"corner determines minimum", bottomcenter.dms().c_str());
		Angle	g = Angle::right_angle - b_lr;
		_mindec = round((g - _gridzero.dec()) / _stepsizes.dec());
	}
	_mindec -= 1;
	if (_mindec < dec_abs_min) {
		_mindec = dec_abs_min;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mindec = %d, maxdec = %d", _mindec, _maxdec);
}

Angle	GridCalculator::ra(int _ra) const {
	return _gridzero.ra() + _ra * _stepsizes.ra();
}

Angle	GridCalculator::dec(int _dec) const {
	return _gridzero.dec() + _dec * _stepsizes.dec();
}

RaDec	GridCalculator::gridpoint(int _ra, int _dec) const {
	return RaDec(ra(_ra),  dec(_dec));
}

TwoAngles	GridCalculator::angleRangeRA(int dec) const {
	if (_pole_in_frame) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "pole in frame");
		return TwoAngles(Angle(0),  Angle(360, Angle::Degrees));
	}
	return TwoAngles(_gridzero.ra() + _minra * _stepsizes.ra(),
			_gridzero.ra() + _maxra * _stepsizes.ra());
}

TwoAngles	GridCalculator::angleRangeDEC(int ra) const {
	return TwoAngles(_gridzero.dec() + _mindec * _stepsizes.dec(),
			_gridzero.dec() + _maxdec * _stepsizes.dec());
}


} // namespace utils
} // namespace astro
