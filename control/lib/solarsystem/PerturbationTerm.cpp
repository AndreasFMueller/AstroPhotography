/*
 * PerturbationTerm.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroSolarsystem.h>

namespace astro {
namespace solarsystem {

/**
 * \brief Construct an individual term of the a perturbation series
 *
 * \param perturbed	the perturbed planetoid
 * \param perturber	the perturbing planetoid
 * \param perturbed_i	the factor for the mean anomaly of the perturbed
 * \param perturber_i	the factor for the mean anomaly of the perturber
 * \param T_exponent	the exponent for the time
 * \param dl_cos	the coefficient of th cos term in l
 * \param dl_sin	the coefficient of th sin term in l
 * \param dr_cos	the coefficient of th cos term in r
 * \param dr_sin	the coefficient of th sin term in r
 * \param db_cos	the coefficient of th cos term in b
 * \param db_sin	the coefficient of th sin term in b
 */
PerturbationTerm::PerturbationTerm(
		const Planetoid& perturbed, const Planetoid& perturber,
                int perturbed_i, int perturber_i, int T_exponent,
                const Angle& dl_cos, const Angle& dl_sin,
                double dr_cos, double dr_sin,
                const Angle& db_cos, const Angle& db_sin)
	: _perturbed(perturbed), _perturber(perturber),
	  _perturbed_i(perturbed_i), _perturber_i(perturber_i),
	  _T_exponent(T_exponent),
	  _dl_cos(dl_cos), _dl_sin(dl_sin),
	  _dr_cos(dr_cos), _dr_sin(dr_sin),
	  _db_cos(db_cos), _db_sin(db_sin) {
}

static inline double	pow(double x, int k) {
	if (k == 0) { return 1.; }
	if (k < 0) { return pow(x, -k); }
	return pow(x, k-1);
}

/**
 * \brief Compute the value of a term
 *
 * \param T	the time in julian centuries
 */
EclipticalCoordinates	PerturbationTerm::operator()(const JulianCenturies& T)
	const {
	SinCos	Mperturbed = _perturbed.Msc(T);
	SinCos	Mperturber = _perturber.Msc(T);
	SinCos	o = Mperturber * _perturber_i + Mperturbed * _perturbed_i;
	return pow((double)T, _T_exponent) * EclipticalCoordinates(
		_dl_cos * o.cos() + _dl_sin * o.sin(),
		_dr_cos * o.cos() + _dr_sin * o.sin(),
		_db_cos * o.cos() + _db_sin * o.sin()
	);
}

} // namespace solarsystem
} // namespace astro
