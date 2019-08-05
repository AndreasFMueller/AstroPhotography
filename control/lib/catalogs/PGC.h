/*
 * PGC.h -- The principal galaxies catalog
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _PGC_h
#define _PGC_h

#include <AstroCatalog.h>
#include <map>
#include <set>
#include <memory>

namespace astro {
namespace catalog {

/**
 * \brief NGC/IC catalog class
 */
class PGC : public std::map<std::string, DeepSkyObject> {
public:
	typedef std::set<DeepSkyObject>	objectset;
	typedef std::shared_ptr<objectset>	objectsetptr;
	std::map<std::string, std::string>	names;
public:
	PGC(const std::string& filename);
	DeepSkyObject	find(const std::string& name) const;
	objectsetptr	find(const SkyWindow& window) const;
	std::set<std::string>	findLike(const std::string& name) const;
};

} // namespace catalog
} // namespace astro

#endif /* _PGC_h */
