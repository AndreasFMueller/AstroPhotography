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
OptimalControl::OptimalControl(CalibrationPtr cal, double deltat)
	: ControlBase(cal, deltat)  {
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
	throw std::runtime_error("optimal control filter not implemented yet");
	// update the filter with the current offset
	_kalmanfilter->update(offset);

	// get the filtered offset
	Point	filtered_offset = _kalmanfilter->offset();

	// correct the filtered offset
	return ControlBase::correct(filtered_offset);
}

} // namespace guiding
} // namespace astro
