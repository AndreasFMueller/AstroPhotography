/*
 * ConstellationCatalog.cpp
 *
 * (c) 2019 Prof Dr Andreas Müller, Hochschule Rapperswi
 */
#include <AstroCatalog.h>
#include "constellations.h"

namespace astro {
namespace catalog {

bool	ConstellationEdge::operator==(const ConstellationEdge& other) const {
	return (from() == other.from()) && (to() == other.to());
}

bool	ConstellationEdge::operator<(const ConstellationEdge& other) const {
	if (from() < other.from()) {
		return true;
	}
	if (from() > other.from()) {
		return false;
	}
	return to() < other.to();
}


/**
 * \brief Compute the centroid of the edges of the constellation
 */
astro::RaDec	Constellation::centroid() const {
	Vector	sum;
	for (auto i = begin(); i != end(); i++) {
		ConstellationEdge	edge = *i;
		sum = sum + UnitVector(edge.from());
		sum = sum + UnitVector(edge.to());
	}
	return RaDec(sum);
}

/**
 * \brief Create a ConstellationCatalog
 */
ConstellationCatalog::ConstellationCatalog() {
	// read the the constellation points
	ConstellationPtr	constellation(NULL);
	for (int i = 0; i < constellation_size; i++) {
		if ((constellation_points[i].name == NULL)
			|| (constellation_points[i+1].name == NULL)) {
			constellation.reset();
			continue;
		}
		// get the constellation of this name
		std::string	name(constellation_points[i].name);
		auto	cptr = find(name);
		if (cptr == end()) {
			constellation = ConstellationPtr(new Constellation(name));
			insert(std::make_pair(name, constellation));
		} else {
			constellation = cptr->second;
		}
		// construct an edge
		RaDec	from(Angle(constellation_points[i].ra, Angle::Hours),
			Angle(constellation_points[i].dec, Angle::Degrees));
		RaDec	to(Angle(constellation_points[i+1].ra, Angle::Hours),
			Angle(constellation_points[i+1].dec, Angle::Degrees));
		constellation->insert(ConstellationEdge(from, to));
	}
}

static ConstellationCatalogPtr      constellationptr(NULL);
static struct std::once_flag    constellation_once;
static void     constellation_initialize() {
        constellationptr = ConstellationCatalogPtr(new ConstellationCatalog());
}

/**
 * \brief Get the constellation catalog
 */
ConstellationCatalogPtr	ConstellationCatalog::get() {
	std::call_once(constellation_once, constellation_initialize);
        return constellationptr;
}

} // namespace catalog
} // namespace astro
