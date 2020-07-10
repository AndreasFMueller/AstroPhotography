/*
 * PerturbedPlanetoid.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswi
 */
#include <AstroSolarsystem.h>

namespace astro {
namespace solarsystem {

/**
 * \brief Perturbed planetoid construction
 *
 * \param name		name of the plantoid
 * \param a		semimajor axis
 * \param e		excentricity
 * \param Omega		ascending node
 * \param i		inclination
 * \param omega		perihelion length
 * \param omegabar	perihelion argument
 * \param n		mean rate
 * \param M0		perihelion offset
 */
PerturbedPlanetoid::PerturbedPlanetoid(const std::string& name,
                double a, double e, const Angle& Omega, const Angle& i, 
                const Angle& omega, const Angle& omegabar, const Angle& n,
                const Angle& M0)
	: Planetoid(name, a, e, Omega, i, omega, omegabar, n, M0) {
}

/**
 * \brief Add a new perturbation series to the planetoid
 *
 * The planetoid can have an arbitrary number of perturbers
 *
 * \param series	the perturbation series to add
 */
void	PerturbedPlanetoid::add(PerturbationSeriesPtr series) {
	_perturbers.push_back(series);
}

/**
 * \brief Summation method
 *
 * \param T	the time in julian centuries
 */
EclipticalCoordinates	PerturbedPlanetoid::ecliptical(
	const JulianCenturies& T) const {
	EclipticalCoordinates	result = Planetoid::ecliptical(T);
	for (auto const s : _perturbers) {
		result = result + s->ecliptical(T);
	}
	return result;
}

} // namespace solarsystem
} // namespace astro
