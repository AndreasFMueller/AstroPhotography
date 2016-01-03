/*
 * CalibrationPersistence.h -- Table containing calibration data
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Calibration_h
#define _Calibration_h

#include <AstroPersistence.h>
#include <AstroGuiding.h>

namespace astro {
namespace guiding {

/**
 * \brief Calibration class 
 */
class Calibration {
public:
	time_t	when;
	std::string	instrument;
	std::string	ccd;
	std::string	guiderport;
	double	a[6];
	Calibration();
};

typedef persistence::Persistent<Calibration>	CalibrationRecord;

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

class CalibrationTable : public astro::persistence::Table<CalibrationRecord, CalibrationTableAdapter> {
public:
	CalibrationTable(astro::persistence::Database& database);
	// select a list of calibrations based on a guider descriptor
	std::list<long>	selectids(const GuiderDescriptor& guiderdescriptor);
	// we must explicitly make the selectids method from the parent
	// visible in the namespace of this class, because it clashes with
	// our new selectids method
	using Table<CalibrationRecord, CalibrationTableAdapter>::selectids;
};

typedef persistence::PersistentRef<CalibrationPoint>	CalibrationPointRecord;

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

typedef astro::persistence::Table<CalibrationPointRecord, CalibrationPointTableAdapter> CalibrationPointTable;

} // namespace guiding
} // namespace astro

#endif /* _Calibration_h */
