/*
 * CalibrationSoruce.h -- common access to calibrations
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _CalibrationSource_h
#define _CalibrationSource_h

#include <AstroPersistence.h>
#include <guider.h>

namespace snowstar {

/**
 * \brief Class implementing access to calibrations
 */
class CalibrationSource {
	astro::persistence::Database	_database;
public:
	CalibrationSource(astro::persistence::Database database);
	CalibrationSource();
	Calibration	get(int id);
};

} // namespace snowstar

#endif /* _CalibrationSource_h */
