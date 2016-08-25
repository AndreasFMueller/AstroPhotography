/*
 * TrackingStore.cpp -- class to get tracking histories from the database
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <AstroDebug.h>
#include <TrackingPersistence.h>
#include <sstream>

using namespace astro::persistence;

namespace astro {
namespace guiding {

/**
 * \brief Retrieve a list of all guide run ids
 *
 * This list is sorted by the start time
 */
std::list<long>	TrackingStore::getAllTrackings() {
	TrackTable	table(_database);
	return table.selectids("order by whenstarted");
}

/**
 * \brief Retrieve a list of all guide run ids matching a desriptor
 *
 * This list is sorted by start time
 */
std::list<long>	TrackingStore::getTrackings(
			const GuiderDescriptor& guider) {
	TrackTable	table(_database);
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
	out << "track = " << id << " order by trackingtime";
	TrackingTable	table(_database);
	return table.select(out.str());
}

/**
 * \brief Retrieve a list of all TrackingPoints
 *
 * The tracking points are sorted by the tracking time attribute
 */
std::list<TrackingPointRecord>	TrackingStore::getHistory(long id,
		ControlDeviceType type) {
	std::ostringstream	out;
	out << "track = " << id;
	switch (type) {
	case GP:
		out << " and controltype = 0";
		break;
	case AO:
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
	TrackTable	table(_database);
	TrackingHistory	history(table.byid(id));
	history.trackid = id;
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
	ControlDeviceType type) {
	TrackTable	table(_database);
	TrackingHistory	history(table.byid(id));
	history.trackid = id;
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
	TrackTable	table(_database);
	if (!table.exists(id)) {
		return;
	}
	table.remove(id);
	std::string	query(	"delete from tracking "
				"where track = ?");
	persistence::StatementPtr	statement = _database->statement(query);
        statement->bind(0, (int)id);
        statement->execute();
}

/**
 * \brief Find out whether a tracking history is contained in the table
 */
bool	TrackingStore::contains(long id) {
	TrackTable	table(_database);
	return table.exists(id);
}

/**
 * \brief Retrive a tracking summary
 */
TrackingSummary	TrackingStore::getSummary(long id) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieveing track %ld summary", id);
	TrackTable	table(_database);
	TrackRecord	track = table.byid(id);
	TrackingSummary	summary(track.name, track.instrument, track.ccd,
		track.guiderport, track.adaptiveoptics);
	summary.trackingid = id;
	summary.starttime = track.whenstarted;
	summary.guiderportcalid = track.guiderportcalid;
	summary.adaptiveopticscalid = track.adaptiveopticscalid;
	// get the summary information about the point data
	std::ostringstream	qstr;
	qstr << "select controltype, count(*), ";
	qstr << "avg(xoffset) as xmean, ";
	qstr << "avg(xoffset * xoffset) as x2mean, ";
	qstr << "avg(yoffset) as ymean, ";
	qstr << "avg(yoffset * yoffset) as y2mean ";
	qstr << "from tracking ";
	qstr << "where track = ? ";
	qstr << "group by controltype ";
	qstr << "order by controltype desc";
	std::string	query = qstr.str();
	persistence::StatementPtr	statement = _database->statement(query);
	statement->bind(0, (int)id);
	Result	result = statement->result();
	if (0 == result.size()) {
		// nothing to summarize
		return summary;
	}
	// process the first row (which is the relevant one
	Row	row = *result.begin();
	int	type = row[0]->intValue(); // we use this later to select last
	int	count = row[1]->intValue();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d points found, type = %d",
		count, type);
	summary.count(count);
	double	xmean = row[2]->doubleValue();
	double	x2mean = row[3]->doubleValue();
	double	ymean = row[4]->doubleValue();
	double	y2mean = row[3]->doubleValue();
	summary.average(Point(xmean, ymean));
	if (count > 1) {
		double	s = count / (count - 1.);
		double	varx = s * (x2mean - xmean * xmean);
		double	vary = s * (y2mean - ymean * ymean);
		summary.variance(Point(varx, vary));
	}

	// now do a query to retrieve the last offset
	query = std::string(
		"select id, xoffset, yoffset "
		"from tracking "
		"where controltype = ? and track = ? "
		"order by id desc "
		"limit 1");
	statement = _database->statement(query);
	statement->bind(0, type);
	statement->bind(1, (int)id);
	result = statement->result();
	if (0 == result.size()) {
		return summary;
	}
	row = *result.begin();
	double	xlast = row[1]->doubleValue();
	double	ylast = row[2]->doubleValue();
	summary.lastoffset = Point(xlast, ylast);

	// Info about data retrieved
	debug(LOG_DEBUG, DEBUG_LOG, 0, "summary %d, points = %d", id,
		summary.count());

	// that's it
	return summary;
}

} // namespace guiding
} // namespace astro
