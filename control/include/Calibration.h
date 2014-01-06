/*
 * Calibration.h -- Table containing calibration data
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Calibration_h
#define _Calibration_h

namespace astro {
namespace guiding {

/**
 * \brief
 */
class Calibration {
	int	_id;
public:
	int	id() const { return _id; }
	void	id(int i) { _id = i; }
	time_t	when;
	double	a[6];
	Calibration() { }
};

/**
 * \brief Table adapter for the Calibration
 */
class CalibrationTableAdapter {
public:
static std::string	tablename();
static std::string	createstatement();
static Calibration
	row_to_object(int objectid, const astro::persistence::Row& row);
static astro::persistence::UpdateSpec
	object_to_updatespec(const Calibration& calibration);
};

typedef astro::persistence::Table<Calibration, CalibrationTableAdapter>	CalibrationTable;

/**
 * \brief calibration raw data
 */
class CalibrationPoint {
	int	_id;
public:
	int	id() const { return _id; }
	int	calibration;
	double	t;
	double	ra;
	double	dec;
	double	x;
	double	y;

	CalibrationPoint(int i) : _id(i) { }
};

/**
 * \brief Table adapter for the calibration points
 */
class CalibrationPointTableAdapter {
public:
static std::string	tablename();
static std::string	createstatement();
static CalibrationPoint
	row_to_object(int objectid, const astro::persistence::Row& row);
static astro::persistence::UpdateSpec
	object_to_updatespec(const CalibrationPoint& calibrationpoint);
};

typedef astro::persistence::Table<CalibrationPoint, CalibrationPointTableAdapter> CalibrationPointTable;

} // namespace guiding
} // namespace astro
