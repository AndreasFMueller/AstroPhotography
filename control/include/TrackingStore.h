/*
 * TrackingStore.h -- class to get tracking histories from the database
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _TrackingStore_h
#define _TrackingStore_h

#include <list>
#include <AstroGuiding.h>
#include <AstroPersistence.h>
#include <TrackingPersistence.h>

namespace astro {
namespace guiding {

class TrackingStore {
	astro::persistence::Database&	_database;
public:
	TrackingStore(astro::persistence::Database& database)
		: _database(database) { }
	std::list<long>	getAllTrackings();
	std::list<long>	getTrackings(const GuiderDescriptor& guider);
	std::list<TrackingPointRecord>	getHistory(long id);
	TrackingHistory	get(long id);
	void	deleteTrackingHistory(long id);
	bool	contains(long id);
};

} // namespace guiding
} // namespace astro

#endif /* _TrackingStore_h */
