/*
 * TrackingStore.cpp -- class to get tracking histories from the database
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <TrackingStore.h>
#include <AstroPersistence.h>
#include <AstroDebug.h>
#include <sstream>

namespace astro {
namespace guiding {

/**
 * \brief Retrieve a list of all guide run ids
 *
 * This list is sorted by the start time
 */
std::list<long>	TrackingStore::getAllTrackings() {
	GuidingRunTable	table(_database);
	return table.selectids("order by whenstarted");
}

/**
 * \brief Retrieve a list of all guide run ids matching a desriptor
 *
 * This list is sorted by start time
 */
std::list<long>	TrackingStore::getTrackings(
			const GuiderDescriptor& guider) {
	GuidingRunTable	table(_database);
	std::ostringstream	out;
	out << "camera = '" << guider.cameraname() << "' and ";
	out << "ccdid = " << guider.ccdid() << " and ";
	out << "guiderport = '" << guider.guiderportname() << "' ";
	out << "order by whenstarted";
	std::string	condition = out.str();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "condition: %s", condition.c_str());
	return table.selectids(condition);
}

/**
 * \brief Retrieve a list of all TrackingPoints
 *
 * The tracking points are sorted by the tracking time attribute
 */
std::list<TrackingPointRecord>	TrackingStore::getHistory(long id) {
	std::ostringstream	out;
	out << "guidingrun = " << id << " order by trackingtime";
	TrackingTable	table(_database);
	return table.select(out.str());
}

/**
 * \brief Get the complete history
 *
 * The tracking history contains the information in both the guiding record
 * as well as all the tracking points.
 */
TrackingHistory	TrackingStore::get(long id) {
	GuidingRunTable	table(_database);
	TrackingHistory	history(table.byid(id));
	std::list<TrackingPointRecord>	track = getHistory(id);
	for (auto ptr = track.begin(); ptr != track.end(); ptr++) {
		history.points.push_back(*ptr);
	}
	return history;
}

} // namespace guiding
} // namespace astro
