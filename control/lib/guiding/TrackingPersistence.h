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

typedef persistence::Persistent<GuidingRun>	GuidingRunRecord;

/**
 * \brief Adapter for GuidingRun table entries
 */
class GuidingRunTableAdapter {
public:
static std::string	tablename();
static std::string	createstatement();
static GuidingRunRecord
	row_to_object(int objectid, const astro::persistence::Row& row);
static astro::persistence::UpdateSpec
	object_to_updatespec(const GuidingRunRecord& guidingrun);
};

typedef astro::persistence::Table<GuidingRunRecord, GuidingRunTableAdapter>	GuidingRunTable;


typedef	persistence::PersistentRef<TrackingPoint>	TrackingPointRecord;

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
