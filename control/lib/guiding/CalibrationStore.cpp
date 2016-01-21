/*
 * CalibrationStore.cpp -- class to get calibration from the database
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CalibrationStore.h>
#include <CalibrationPersistence.h>
#include <sstream>
#include <AstroDebug.h>

namespace astro {
namespace guiding {

/**
 * \brief Get a list of all calibrations
 *
 * This method retrieves the calibrations in a sorted way
 */
std::list<long>	CalibrationStore::getAllCalibrations() {
	CalibrationTable	table(_database);
	return table.selectids(std::string("order by whenstarted"));
}

/**
 * \brief Get a list of all calibrations of a certain guider
 *
 * This method retrieves all calibrations in the database that match
 * the guider descriptor, sorted by start time.
 */
std::list<long>	CalibrationStore::getCalibrations(
			const GuiderDescriptor& guider) {
	std::ostringstream	out;
	out << " instrument = '" << guider.instrument() << "' and ";
	out << " ccd = '" << guider.ccd() << "' and ";
	out << " controldevice = '" << guider.guiderport() << "'";
	out << " order by whenstarted";
	std::string	condition = out.str();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "condition: %s", condition.c_str());
	CalibrationTable	table(_database);
	return table.selectids(condition);
}

/**
 * \brief Get the calibration points for a given calibration
 *
 * The points are sorted by the time argument
 */
std::list<CalibrationPointRecord>	CalibrationStore::getCalibrationPoints(long id) {
	std::ostringstream	out;
	out << "calibration = " << id << " order by t";
	CalibrationPointTable	table(_database);
	return table.select(out.str());
}

/**
 * \brief Get the complete calibration
 *
 * \param id of the calibration to retrieve
 */
GuiderCalibration	CalibrationStore::getCalibration(long id) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieving calibration %d", id);

	// get the calibration table
	CalibrationTable	ct(_database);
	CalibrationRecord	r = ct.byid(id);
	GuiderCalibration	calibration;
	for (int i = 0; i < 6; i++) {
		calibration.a[i] = r.a[i];
	}
	calibration.focallength = r.focallength;
	calibration.complete((r.complete) ? true : false);
	calibration.masPerPixel = r.masPerPixel;
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"found calibration with masPerPixel=%.3f", r.masPerPixel);
	switch (r.controltype) {
	case 0:	calibration.calibrationtype(BasicCalibration::GP);
		break;
	case 1:	calibration.calibrationtype(BasicCalibration::AO);
		break;
	}

	// add the points
	std::list<CalibrationPointRecord>	points
		= getCalibrationPoints(id);
	std::list<CalibrationPointRecord>::const_iterator	i;
	for (i = points.begin(); i != points.end(); i++) {
		CalibrationPoint	p = *i;
		calibration.push_back(p);
	}
	return calibration;
}

/**
 * \brief remove a calibration together with the points
 */
void	CalibrationStore::deleteCalibration(long id) {
	CalibrationTable	ct(_database);
	if (!ct.exists(id)) {
		return;
	}
	ct.remove(id);
	removePoints(id);
}

/**
 * \brief Add a calibration to the database
 */
long	CalibrationStore::addCalibration(const PersistentCalibration& calibration) {
	CalibrationTable	t(_database);
	CalibrationRecord	record(0, calibration);
	return t.add(record);
}

/**
 * \brief update a calibration record in the database
 */
void	CalibrationStore::updateCalibration(long id,
		const GuiderCalibration& calibration) {
	CalibrationTable	t(_database);
	CalibrationRecord	record = t.byid(id);
	for (int i = 0; i < 6; i++) {
		record.a[i] = calibration.a[i];
	}
	record.focallength = calibration.focallength;
	record.det = calibration.det();
	record.quality = calibration.quality();
	record.complete = (calibration.complete()) ? 1 : 0;
	record.masPerPixel = calibration.masPerPixel;
	t.update(id, record);
}

/**
 * \brief Add a point to an existing calibration process
 */
void	CalibrationStore::addPoint(long id, const CalibrationPoint& point) {
	CalibrationPointRecord	record(0, id, point);
	CalibrationPointTable	t(_database);
	t.add(record);
}

/**
 * \brief remove all points that belong to a calibration identified by an id
 */
void	CalibrationStore::removePoints(long id) {
	std::string	query(  "delete from calibrationpoint "
                                "where calibration = ?");
        persistence::StatementPtr    statement = _database->statement(query);
        statement->bind(0, (int)id);
        statement->execute();
}

/**
 * \brief Find out whether a calibration exists in the store
 */
bool	CalibrationStore::contains(long id) {
	CalibrationTable	ct(_database);
	return ct.exists(id);
}

/**
 * \brief Save a basic calibration in the database
 *
 * This method adds the calibration data to an already existing calibration
 * record in the database
 */
void	CalibrationStore::saveCalibration(long id, const BasicCalibration& cal) {
	GuiderCalibration	c = getCalibration(id);
	c.complete(true);
	c = cal;
	updateCalibration(id, c);
	removePoints(id);
	for (unsigned int i = 0; i < cal.size(); i++) {
		CalibrationPoint	point = cal[i];
		addPoint(id, point);
	}
}

} // namespace guiding
} // namespace astro
