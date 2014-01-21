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

class CalibrationStore {
	astro::persistence::Database&	_database;
public:
	CalibrationStore(astro::persistence::Database& database)
		: _database(database) { }
	std::list<long>	getAllCalibrations();
	std::list<long>	getCalibrations(const GuiderDescriptor& guider);
	std::list<CalibrationPointRecord>	getCalibrationPoints(long id);
};

} // namespace guiding
} // namespace astro

#endif /* _CalibrationStore_h */
