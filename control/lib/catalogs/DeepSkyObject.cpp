/*
 * DeepSkyObject.cpp -- DeepSkyObject implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCatalog.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <includes.h>
#include "CatalogBackend.h"
#include "CatalogIterator.h"
#include <typeinfo>

namespace astro {
namespace catalog {

//////////////////////////////////////////////////////////////////////
// DeepSkyObject implementation
//////////////////////////////////////////////////////////////////////
/**
 * \brief string representation of a DeepSkyObject
 */
std::string	DeepSkyObject::toString() const {
	return stringprintf("%s: %s %s %.2f (%s)", name.c_str(),
			ra().hms().c_str(), dec().dms().c_str(), mag(),
			constellation.c_str());
}

std::string      DeepSkyObject::classification2string(DeepSkyObject::object_class c) {
	switch (c) {
	case Galaxy:
		return std::string("galaxy");
	case OpenCluster:
		return std::string("open cluster");
	case GlobularCluster:
		return std::string("globular cluster");
	case BrightNebula:
		return std::string("bright nebula");
	case PlanetaryNebula:
		return std::string("planetary nebula");
	case ClusterNebulosity:
		return std::string("cluster with nebulosity");
	case Asterism:
		return std::string("asterism");
	case Knot:
		return std::string("knot");
	case TripleStar:
		return std::string("triple star");
	case DoubleStar:
		return std::string("double star");
	case SingleStar:
		return std::string("single star");
	case Uncertain:
		return std::string("uncertain");
	case Unidentified:
		return std::string("unidentified");
	case Nonexistent:
		return std::string("nonexistent");
	case PlateDefect:
		return std::string("plate defect");
	}
}

DeepSkyObject::object_class   DeepSkyObject::string2classification(const std::string& s) {
	if (s == "galaxy") {
		return Galaxy;
	}
	if (s == "open cluster") {
		return OpenCluster;
	}
	if (s == "globular cluster") {
		return GlobularCluster;
	}
	if (s == "bright nebula") {
		return BrightNebula;
	}
	if (s == "planetary nebula") {
		return PlanetaryNebula;
	}
	if (s == "cluster with nebulosity") {
		return ClusterNebulosity;
	}
	if (s == "asterism") {
		return Asterism;
	}
	if (s == "knot") {
		return Knot;
	}
	if (s == "triple star") {
		return TripleStar;
	}
	if (s == "double star") {
		return DoubleStar;
	}
	if (s == "single star") {
		return SingleStar;
	}
	if (s == "uncertain") {
		return Uncertain;
	}
	if (s == "unidentified") {
		return Unidentified;
	}
	if (s == "nonexistent") {
		return Nonexistent;
	}
	if (s == "plate defect") {
		return PlateDefect;
	}
}

} // namespace catalog
} // namespace astro
