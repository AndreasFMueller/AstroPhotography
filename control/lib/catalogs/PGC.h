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
#include <sqlite3.h>

namespace astro {
namespace catalog {

class PGCDatabase {
	std::string	_dbfilename;
	sqlite3		*_database;
	sqlite3_stmt	*_insert_stmt;
public:
	PGCDatabase(const std::string& dbfilename);
	~PGCDatabase();
	const std::string&	dbfilename() const { return _dbfilename; }
	void	add(const DeepSkyObject& deepskyobject);
	DeepSkyObject	find(const std::string& name);
	DeepSkyObjectSetPtr	find(const SkyWindow& window);
	std::set<std::string>	findLike(const std::string& name,
					size_t maxobjects = 100);
	size_t	size();
};

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
	DeepSkyObjectSetPtr	find(const SkyWindow& window) const;
	std::set<std::string>	findLike(const std::string& name,
					size_t maxobjects = 100) const;
};

} // namespace catalog
} // namespace astro

#endif /* _PGC_h */
