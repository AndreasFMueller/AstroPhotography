/*
 * CalibrationStore.h -- class to get calibrations from the database
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _CalibrationStore_h
#define _CalibrationStore_h

#include <list>
#include <AstroGuiding.h>
#include <AstroPersistence.h>
#include <CalibrationPersistence.h>

namespace astro {
namespace guiding {

/**
 * \brief a simplified interface to the calibration persistence  tables
 */
class CalibrationStore {
	astro::persistence::Database&	_database;
public:
	CalibrationStore(astro::persistence::Database& database)
		: _database(database) { }
	std::list<long>	getAllCalibrations();
	std::list<long>	getCalibrations(const GuiderDescriptor& guider);

	// access to calibrations
	bool	contains(long id);
	GuiderCalibration	getCalibration(long id);
	long	addCalibration(const Calibration& calibration);
	void	updateCalibration(long id, const GuiderCalibration& calibration);
	void	deleteCalibration(long id);

	// guider points
	std::list<CalibrationPointRecord>	getCalibrationPoints(long id);
	void	addPoint(long id, const CalibrationPoint& point);
};

} // namespace guiding
} // namespace astro

#endif /* _CalibrationStore_h */
