/*
 * TrackingPersistence.h -- Table containing tracking log data
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Tracking_h
#define _Tracking_h

#include <string>
#include <AstroPersistence.h>
#include <AstroGuiding.h>

namespace astro {
namespace guiding {

#if 0
typedef persistence::Persistent<Track>	TrackRecord;
#endif

/**
 * \brief Adapter for Track table entries
 */
class TrackTableAdapter {
public:
static std::string	tablename();
static std::string	createstatement();
static TrackRecord
	row_to_object(int objectid, const astro::persistence::Row& row);
static astro::persistence::UpdateSpec
	object_to_updatespec(const TrackRecord& guidingrun);
};

typedef astro::persistence::Table<TrackRecord, TrackTableAdapter>	TrackTable;


#if 0
typedef	persistence::PersistentRef<TrackingPoint>	TrackingPointRecord;
#endif

/**
 * \brief Adapter for the Tracking table
 */
class TrackingTableAdapter {
public:
static std::string	tablename();
static std::string	createstatement();
static TrackingPointRecord	row_to_object(int objectid, const astro::persistence::Row& row);
static astro::persistence::UpdateSpec	object_to_updatespec(const TrackingPointRecord& tracking);
};

typedef astro::persistence::Table<TrackingPointRecord, TrackingTableAdapter>	TrackingTable;

} // namespace guiding
} // namespace astro

#endif /* _Tracking_h */
