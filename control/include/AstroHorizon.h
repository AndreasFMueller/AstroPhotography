/*
 * AstroHorizon.h
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _AstroHorizon_h
#define _AstroHorizon_h

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
	void	addbasepoint();
public:
	Horizon();
	Horizon(const Angle& alt);
	Horizon(const std::string& filename);
	Horizon(const Horizon& other, const Angle& angle);
	void	addgrid(const Angle& gridconstant);
	HorizonPtr	rotate(const Angle& angle) const;
	static HorizonPtr	get();
	static HorizonPtr	get(const std::string& filename);
	static HorizonPtr	get(const std::string& filename,
		const Angle& angle);
};

} // namespace horizon
} // namespace astro

#endif /* _AstroHorizon_h */
