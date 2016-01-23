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
	std::list<long>	getAllCalibrations(BasicCalibration::CalibrationType);
	std::list<long>	getCalibrations(const GuiderDescriptor& guider);

	// access to calibrations
	bool	contains(long id);
	bool	contains(long id, BasicCalibration::CalibrationType type);
	bool	containscomplete(long id, BasicCalibration::CalibrationType type);
	long	addCalibration(const PersistentCalibration& calibration);
	void	deleteCalibration(long id);
	void	updateCalibration(const BasicCalibration& calibration);

	// guider calibration
	GuiderCalibration	getGuiderCalibration(long id);
	void	updateCalibration(const GuiderCalibration& calibration);

	// adaptive optics calibration
	AdaptiveOpticsCalibration	getAdaptiveOpticsCalibration(long id);
	void	updateCalibration(const AdaptiveOpticsCalibration& calibration);

	// guider points
	std::list<CalibrationPointRecord>	getCalibrationPoints(long id);
	void	addPoint(long id, const CalibrationPoint& point);
	void	removePoints(long id);

	// storing basic calibrations, i.e. just the raw calibration data
	// without all the attributes
	void	saveCalibration(const BasicCalibration& cal);
};

} // namespace guiding
} // namespace astro

#endif /* _CalibrationStore_h */
