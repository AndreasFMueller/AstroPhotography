/*
 * NGCIC.h -- The NGC/IC deep sky object catalog
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _NGCIC_h
#define _NGCIC_h

#include <AstroCatalog.h>
#include <map>
#include <set>
#include <memory>

namespace astro {
namespace catalog {

/**
 * \brief NGC/IC catalog class
 */
class NGCIC : std::map<std::string, DeepSkyObject> {
public:
	typedef std::set<DeepSkyObject>	objectset;
	typedef std::shared_ptr<objectset>	objectsetptr;
public:
	NGCIC(const std::string& filename);
	DeepSkyObject	find(const std::string& name) const;
	objectsetptr	find(const SkyWindow& window) const;
};

} // namespace catalog
} // namespace astro

#endif /* _NGCIC_h */
