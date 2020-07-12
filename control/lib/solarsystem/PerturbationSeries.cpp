/*
 * PerturbationSeries.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroSolarsystem.h>


namespace astro {
namespace solarsystem {

/**
 * \brief Create a new perturbation series
 *
 * \param perturbed	the Planetoid being perturbed
 * \param perturber	the perturbing planetoid
 */
PerturbationSeries::PerturbationSeries(const Planetoid& perturbed,
	const Planetoid& perturber)
	: _perturbed(perturbed), _perturber(perturber) {
}

/**
 * \brief Add a term of the perturbation series
 *
 * \param perturbed_i	the factor for the mean anomaly of the perturbed
 * \param perturber_i	the factor for the mean anomaly of the perturber
 * \param T_exponent	the exponent of T in the term
 * \param dl_cos	cos coefficient for l perturbation
 * \param dl_sin	sin coefficient for l perturbation
 * \param dr_cos	cos coefficient for r perturbation
 * \param dr_sin	sin coefficient for r perturbation
 * \param db_cos	cos coefficient for b perturbation
 * \param db_sin	sin coefficient for b perturbation
 */
PerturbationTerm        PerturbationSeries::add(
			int perturbed_i, int perturber_i, int T_exponent,
			const Angle& dl_cos, const Angle& dl_sin,
			double dr_cos, double dr_sin,
			const Angle& db_cos, const Angle& db_sin) {
	PerturbationTerm	newterm(_perturbed, _perturber,
		perturbed_i, perturber_i, T_exponent,
		dl_cos, dl_sin, dr_cos, dr_sin, db_cos, db_sin);
	push_back(newterm);
	return newterm;
}

/**
 * \brief Add a term to the perturbation series
 *
 * \param perturbed_i	the factor for the mean anomaly of the perturbed
 * \param perturber_i	the factor for the mean anomaly of the perturber
 * \param T_exponent	the exponent of T in the term
 * \param dl_cos	cos coefficient for l perturbation
 * \param dl_sin	sin coefficient for l perturbation
 * \param dr_cos	cos coefficient for r perturbation
 * \param dr_sin	sin coefficient for r perturbation
 * \param db_cos	cos coefficient for b perturbation
 * \param db_sin	sin coefficient for b perturbation
 */
PerturbationTerm	PerturbationSeries::add(
				int perturbed_i, int perturber_i,
				int T_exponent,
				double dl_cos, double dl_sin,
				double dr_cos, double dr_sin,
				double db_cos, double db_sin) {
	return add(perturbed_i, perturber_i, T_exponent,
			Angle(dl_cos, Angle::ArcSeconds),
			Angle(dl_sin, Angle::ArcSeconds),
			dr_cos * 1e-5, dr_sin * 1e-5,
			Angle(db_cos, Angle::ArcSeconds),
			Angle(db_sin, Angle::ArcSeconds));
}

/**
 * \brief Sum the series
 *
 * \param T	the time in julian centuries
 */
EclipticalCoordinates   PerturbationSeries::perturbations(
	const JulianCenturies& T) const {
	EclipticalCoordinates	result;
	for (const auto t : *this) {
		result = result + t(T);
	}
	return result;
}

/**
 * \brief Sum the series
 *
 * \param T	the time in julian centuries
 */
EclipticalCoordinates	PerturbationSeries::operator()(
	const JulianCenturies& T) const {
	return perturbations(T);
}

} // namespace solarsystem
} // namespace astro
