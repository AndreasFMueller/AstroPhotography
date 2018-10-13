/*
 * DeepSkyCatalogs.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _DeepSkyCatalogs_h
#define _DeepSkyCatalogs_h

#include <AstroCatalog.h>
#include "NGCIC.h"

namespace astro {
namespace catalog {

/**
 * \brief Messier Catalog
 */
class MessierCatalog : public DeepSkyCatalog {
public:
	MessierCatalog(const std::string& basedir) : DeepSkyCatalog(basedir) { }
	virtual deepskyobjectsetptr	find(const SkyWindow&);
	virtual DeepSkyObject	find(const std::string& name);
	virtual std::set<std::string>	findLike(const std::string& name);
};

/**
 * \brief NGC/IC catalog
 */
class NGCICCatalog : public DeepSkyCatalog, public NGCIC {
public:
	NGCICCatalog(const std::string& basedir);
	virtual deepskyobjectsetptr	find(const SkyWindow&);
	virtual DeepSkyObject	find(const std::string& name);
	virtual std::set<std::string>	findLike(const std::string& name);
};

} // namespace catalog
} // namespace astro

#endif /* _DeepSkyCatalogs_h */
