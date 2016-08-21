/*
 * CalibrationPersistence.h -- Table containing calibration data
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _CalibrationPersistence_h
#define _CalibrationPersistence_h

#include <AstroPersistence.h>
#include <AstroGuiding.h>

namespace astro {
namespace guiding {

typedef persistence::Persistent<PersistentCalibration>	CalibrationRecord;
typedef std::shared_ptr<CalibrationRecord>	CalibrationRecordPtr;

/**
 * \brief Table adapter for the Calibration
 */
class CalibrationTableAdapter {
public:
static std::string	tablename();
static std::string	createstatement();
static CalibrationRecord
	row_to_object(int objectid, const astro::persistence::Row& row);
static astro::persistence::UpdateSpec
	object_to_updatespec(const CalibrationRecord& calibration);
};

/**
 * \brief Table for Calibration information
 */
class CalibrationTable : public astro::persistence::Table<CalibrationRecord,
	CalibrationTableAdapter> {
public:
	CalibrationTable(astro::persistence::Database& database);
	// select a list of calibrations based on a guider descriptor
	std::list<long>	selectids(const GuiderDescriptor& guiderdescriptor);
	// we must explicitly make the selectids method from the parent
	// visible in the namespace of this class, because it clashes with
	// our new selectids method
	using Table<CalibrationRecord, CalibrationTableAdapter>::selectids;
};

/**
 * \brief Table adapter for the calibration points
 */
class CalibrationPointTableAdapter {
public:
static std::string	tablename();
static std::string	createstatement();
static CalibrationPointRecord
	row_to_object(int objectid, const astro::persistence::Row& row);
static astro::persistence::UpdateSpec
	object_to_updatespec(const CalibrationPointRecord& calibrationpoint);
};

/**
 * \brief Table for Calibration points
 */
typedef astro::persistence::Table<CalibrationPointRecord,
		CalibrationPointTableAdapter> CalibrationPointTable;

} // namespace guiding
} // namespace astro

#endif /* _CalibrationPersistence_h */
