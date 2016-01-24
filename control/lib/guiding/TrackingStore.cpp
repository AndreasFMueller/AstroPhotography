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
	out << "instrument = '" << guider.instrument() << "' and ";
	out << "ccd = '" << guider.ccd() << "' and ";
	out << "guiderport = '" << guider.guiderport() << "' ";
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
 * \brief Retrieve a list of all TrackingPoints
 *
 * The tracking points are sorted by the tracking time attribute
 */
std::list<TrackingPointRecord>	TrackingStore::getHistory(long id,
		BasicCalibration::CalibrationType type) {
	std::ostringstream	out;
	out << "guidingrun = " << id;
	switch (type) {
	case BasicCalibration::GP:
		out << " and controltype = 0";
		break;
	case BasicCalibration::AO:
		out << " and controltype = 1";
		break;
	}
	out << " order by trackingtime";
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

/**
 * \brief Get the complete history
 *
 * The tracking history contains the information in both the guiding record
 * as well as all the tracking points.
 */
TrackingHistory	TrackingStore::get(long id,
	BasicCalibration::CalibrationType type) {
	GuidingRunTable	table(_database);
	TrackingHistory	history(table.byid(id));
	std::list<TrackingPointRecord>	track = getHistory(id, type);
	for (auto ptr = track.begin(); ptr != track.end(); ptr++) {
		history.points.push_back(*ptr);
	}
	return history;
}

/**
 * \brief Delete the tracking history
 */
void	TrackingStore::deleteTrackingHistory(long id) {
	GuidingRunTable	table(_database);
	if (!table.exists(id)) {
		return;
	}
	table.remove(id);
	std::string	query(	"delete from tracking "
				"where guidingrun = ?");
	persistence::StatementPtr	statement = _database->statement(query);
        statement->bind(0, (int)id);
        statement->execute();
}

/**
 * \brief Find out whether a tracking history is contained in the table
 */
bool	TrackingStore::contains(long id) {
	GuidingRunTable	table(_database);
	return table.exists(id);
}

} // namespace guiding
} // namespace astro
