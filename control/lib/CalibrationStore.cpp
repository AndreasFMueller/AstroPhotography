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
	out << " camera = '" << guider.cameraname() << "' and ";
	out << " ccdid = " << guider.ccdid() << " and ";
	out << " cameraport = " << guider.guiderportname();
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
	GuiderCalibration	calibration;

	// get the calibration table
	CalibrationTable	ct(_database);
	CalibrationRecord	r = ct.byid(id);
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

} // namespace guiding
} // namespace astro
