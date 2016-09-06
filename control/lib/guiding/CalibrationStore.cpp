/*
 * CalibrationStore.cpp -- class to get calibration from the database
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "CalibrationPersistence.h"
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
 * \brief Get a list of all calibrations of a given type
 *
 * This method retrieves all the calibrations for devices of the type
 * specified in the type argument, sorted by id
 */
std::list<long>	CalibrationStore::getAllCalibrations(ControlDeviceType type) {
	CalibrationTable	table(_database);
	switch (type) {
	case GP:
		return table.selectids(std::string("where controltype = 0 "
						   "order by whenstarted"));
	case AO:
		return table.selectids(std::string("where controltype = 1 "
						   "order by whenstarted"));
	}
	return std::list<long>();
}

/**
 * \brief Get a list of all calibrations of a certain guider
 *
 * This method retrieves all calibrations in the database that match
 * the guider descriptor, sorted by start time.
 */
std::list<long>	CalibrationStore::getCalibrations(
			const GuiderDescriptor& guider,
			ControlDeviceType type) {
	std::ostringstream	out;
	out << " name = '" << guider.name() << "'";
	out << " and controltype = ";
	switch (type) {
	case GP:	out << 0; break;
	case AO:	out << 1; break;
	}
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
CalibrationPtr	CalibrationStore::getCalibration(long id) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieving calibration %d", id);

	// get the calibration table
	CalibrationTable	ct(_database);
	CalibrationRecord	r = ct.byid(id);

	// construct the name from the calibration record retrived
	ControlDeviceName	cdname = nameFromRecord(r);

	// now create the object
	BasicCalibration	*calibration = NULL;
	switch (cdname.controldevicetype()) {
	case GP:
		calibration = new GuiderCalibration(cdname);
		break;
	case AO:
		calibration = new AdaptiveOpticsCalibration(cdname);
		break;
	}
	CalibrationPtr	result(calibration);

	// construct the calibration object
	calibration->calibrationid(id);
	calibration->when(r.when);
	for (int i = 0; i < 6; i++) {
		calibration->a[i] = r.a[i];
	}
	calibration->complete((r.complete) ? true : false);
	calibration->focallength = r.focallength;
	calibration->masPerPixel = r.masPerPixel;
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"found calibration with masPerPixel=%.3f", r.masPerPixel);

	// add the points
	std::list<CalibrationPointRecord>	points
		= getCalibrationPoints(id);
	std::list<CalibrationPointRecord>::const_iterator	i;
	for (i = points.begin(); i != points.end(); i++) {
		CalibrationPoint	p = *i;
		calibration->push_back(p);
	}
	return result;
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
void	CalibrationStore::updateCalibration(
		const BasicCalibration& calibration) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "update calibration %d",
		calibration.calibrationid());
	CalibrationTable	t(_database);
	CalibrationRecord	record = t.byid(calibration.calibrationid());
	for (int i = 0; i < 6; i++) {
		record.a[i] = calibration.a[i];
	}
	record.det = calibration.det();
	record.quality = calibration.quality();
	record.complete = (calibration.complete()) ? 1 : 0;
	record.focallength = calibration.focallength;
	record.masPerPixel = calibration.masPerPixel;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "quality = %f", record.quality);
	t.update(calibration.calibrationid(), record);
}

/**
 * \brief Add a point to an existing calibration process
 */
void	CalibrationStore::addPoint(long id, const CalibrationPoint& point) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add %s to %d",
		point.toString().c_str(), id);
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
 * \brief Find out whether a calibration exists in the store
 */
bool	CalibrationStore::contains(long id, ControlDeviceType type) {
	CalibrationTable	ct(_database);
	std::string	condition;
	switch (type) {
	case GP:
		condition = stringprintf("controltype = 0 and id = %d", id);
		break;
	case AO:
		condition = stringprintf("controltype = 1 and id = %d", id);
		break;
	}
	return (ct.selectids(condition).size() > 0);
}

/**
 * \brief Find out whether a calibration exists that in addition is complete
 */
bool	CalibrationStore::containscomplete(long id, ControlDeviceType type) {
	CalibrationTable	ct(_database);
	std::ostringstream	str;
	str << " complete = 1 "; // ensure we only get complete calibrations
	switch (type) {
	case GP:
		str << "and controltype = 0 and id = " << id;
		break;
	case AO:
		str << "and controltype = 1 and id = " << id;
		break;
	}
	std::string	condition = str.str();
	return (ct.selectids(condition).size() > 0);
}

/**
 * \brief Save a basic calibration in the database
 *
 * This method adds the calibration data to an already existing calibration
 * record in the database
 */
void	CalibrationStore::saveCalibration(const BasicCalibration& cal) {
	int	id = cal.calibrationid();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "saving calibration %d", id);
	CalibrationPtr	c = getCalibration(id);
	c->complete(true);
	*c = cal;
	updateCalibration(*c);

	// add the points
	removePoints(id);
	for (unsigned int i = 0; i < cal.size(); i++) {
		CalibrationPoint	point = cal[i];
		addPoint(cal.calibrationid(), point);
	}
}

/**
 * \brief Get the name from 
 */
ControlDeviceName	CalibrationStore::nameFromRecord(
		const CalibrationRecord& record) const {
	GuiderName	guidername(record.name);
	switch (record.controltype) {
	case 0: return ControlDeviceName(guidername, GP);
	case 1: return ControlDeviceName(guidername, AO);
	}
	std::string	msg = stringprintf("invalid control device type %d",
		record.controltype);
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

} // namespace guiding
} // namespace astro
