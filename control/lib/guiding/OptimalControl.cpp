/*
 * OptimalControl.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <Control.h>
#include <KalmanFilter.h>

namespace astro {
namespace guiding {

/**
 * \brief Create an optimal controller object
 */
OptimalControl::OptimalControl(double deltat) : ControlBase(deltat)  {
	_kalmanfilter = new KalmanFilter(deltat);
}

/**
 * \brief Destroy the optimal control object
 */
OptimalControl::~OptimalControl() {
	delete _kalmanfilter;
}

/**
 * \brief Correct with the current offset
 */
Point	OptimalControl::correct(const Point& offset) {
	// update the filter with the current offset
	_kalmanfilter->update(offset);

	// get the filtered offset
	Point	filtered_offset = _kalmanfilter->offset();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Kalman: offset=%s, filtered=%s",
		offset.toString().c_str(), filtered_offset.toString().c_str());

	// correct the filtered offset
	return ControlBase::correct(filtered_offset);
}

/**
 * \brief set measurement error
 */
void	OptimalControl::measurementerror(double m) {
	_kalmanfilter->measurementerror(m);
}

/**
 * \brief set system error
 */
void	OptimalControl::systemerror(double s) {
	_kalmanfilter->systemerror(s);
}

/**
 * \brief get measurement error
 */
double	OptimalControl::measurementerror() const {
	return _kalmanfilter->measurementerror();
}

/**
 * \brief get system error
 */
double	OptimalControl::systemerror() const {
	return _kalmanfilter->systemerror();
}

/**
 * \brief override the parameter method to set the error in the kalman filter
 */
void	OptimalControl::filter_parameter(int index, double value) {
	ControlBase::filter_parameter(index, value);
	switch (index) {
	case 0: systemerror(value); break;
	case 1: measurementerror(value); break;
	}
}

} // namespace guiding
} // namespace astro
