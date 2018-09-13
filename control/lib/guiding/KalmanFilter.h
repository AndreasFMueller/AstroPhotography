/*
 * KalmanFilter.h -- Declaration of the Kalman-Filter
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _KalmanFilter_h
#define _KalmanFilter_h

#include <AstroTypes.h>
#include <Algebra.h>

namespace astro {
namespace guiding {

/**
 * \brief Kalman filter used to filter noise from pixel positions
 *
 * The optimal control algorithm needs to correct Kalman filtered
 * offsets. This class implements the Kalman filter needed for
 * the purpose.
 */
class KalmanFilter {
	double	_deltat;
	Vector<double,4>	x;
	Matrix<double,2,4>	H;
	Matrix<double,4,4>	P;
	Matrix<double,4,4>	Q;
	Matrix<double,2,2>	R;
	Matrix<double,4,4>	phi;
	Matrix<double,4,2>	K;
public:
	KalmanFilter(double deltat);
	Point	offset() const;
	void	update(const Point& o);
};

} // namespace guiding
} // namespace astro

#endif /* _KalmanFilter_h */
