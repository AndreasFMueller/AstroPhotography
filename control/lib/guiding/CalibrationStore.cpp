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
	out << " guiderport = '" << guider.guiderport() << "'";
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

	// get the calibration table
	CalibrationTable	ct(_database);
	CalibrationRecord	r = ct.byid(id);
	GuiderCalibration	calibration;
	for (int i = 0; i < 6; i++) {
		calibration.a[i] = r.a[i];
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
 * \brief Add a calibration to the database
 */
long	CalibrationStore::addCalibration(const Calibration& calibration) {
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


} // namespace guiding
} // namespace astro
