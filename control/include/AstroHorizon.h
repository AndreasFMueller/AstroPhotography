/*
 * AstroHorizon.h
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroCoordinates.h>
#include <set>
#include <memory>

namespace astro {
namespace horizon {

class Horizon;
typedef std::shared_ptr<Horizon>	HorizonPtr;

/**
 * \brief Horizon object
 *
 * This object expects data in the form of a CSV file as produced by 
 * the iOS theodolite app.
 */
class Horizon : public std::set<AzmAlt> {
public:
	Horizon();
	Horizon(const std::string& filename);
	static HorizonPtr	get();
};

} // namespace horizon
} // namespace astro
