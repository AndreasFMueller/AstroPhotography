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
 * \param n		mean rate
 * \param M0		perihelion offset
 */
PerturbedPlanetoid::PerturbedPlanetoid(const std::string& name,
                double a, double e, const Angle& Omega, const Angle& i, 
                const Angle& omega, const Angle& n, const Angle& M0)
	: Planetoid(name, a, e, Omega, i, omega, n, M0) {
}

/**
 * \brief Construct a perturbation series planetoid from a simple planetoid
 *
 * \param planetoid	the simple planetoid to copy the data from
 */
PerturbedPlanetoid::PerturbedPlanetoid(const Planetoid& planetoid)
	: Planetoid(planetoid) {
}

PerturbedPlanetoid::~PerturbedPlanetoid() {
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
	EclipticalCoordinates	result = this->position(T);
	result = result + this->perturbations(T) + this->corrections(T);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "perturbed %s: %s", this->name().c_str(),
		result.toString().c_str());
	return result;
}

/**
 * \brief compute the perturbations specifically
 *
 * \param T	the time in julian centuries
 */
EclipticalCoordinates	PerturbedPlanetoid::perturbations(
	const JulianCenturies& T) const {
	EclipticalCoordinates	result;
	for (auto const s : _perturbers) {
		result = result + s->perturbations(T);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "perturbations: %s", result.toString().c_str());
	return result;
}

/**
 * \brief Compute additional corrections that are not in the form of perturbation series
 *
 * \param T	the time in julian centuries
 */
EclipticalCoordinates	PerturbedPlanetoid::corrections(
				const JulianCenturies& /* T */) const {
	return EclipticalCoordinates(Angle(0), 0, Angle(0));
}

} // namespace solarsystem
} // namespace astro
