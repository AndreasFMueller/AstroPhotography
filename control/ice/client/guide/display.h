/*
 * display.h -- classes for display 
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _display_h
#define _display_h

#include <iostream>
#include <guider.h>

namespace snowstar {
namespace app {
namespace snowguide {

/**
 * \brief A class to display calibration points
 */
class CalibrationPoint_display {
	std::ostream&	_out;
public:
	CalibrationPoint_display(std::ostream& out);
	void	operator()(const CalibrationPoint& calpoint);
};

/**
 * \brief A class to display a calibration
 */
class Calibration_display {
	std::ostream&	_out;
	bool	_verbose;
public:
	bool	verbose() const { return _verbose; }
	void	verbose(bool v) { _verbose = v; }

	Calibration_display(std::ostream& out);
	void	operator()(const Calibration& cal);
};

/**
 * \brief Tracking point display class
 *
 * This class is a functor class used to display a tracking point. It also
 * keeps track of the index of the point being displayed.
 */
class TrackingPoint_display {
	std::ostream&	_out;
	int	counter;
	double	_starttime;
	bool	_csv;
public:
	bool	csv() const { return _csv; }
	void	csv(bool c) { _csv = c; }
private:
	double	_masperpixel;
public:
	double	masperpixel() const { return _masperpixel; }
	void	masperpixel(double m) { _masperpixel = m; }
public:
	TrackingPoint_display(std::ostream& out, double starttime);
	void	operator()(const TrackingPoint& point);
};

/**
 * \brief Display of a tracking history
 */
class TrackingHistory_display {
	GuiderFactoryPrx	_guiderfactory;
	bool	_verbose;
	bool	_csv;
public:
	TrackingHistory_display(GuiderFactoryPrx guiderfactory,
		bool verbose, bool csv)
		: _guiderfactory(guiderfactory), _verbose(verbose), _csv(csv) {
	}
	void	operator()(const TrackingHistory history);
};

} // namespace snowguide
} // namespace app
} // namespace snowstar

#endif /* _display_h */
