/*
 * KalmanFilter.cpp -- Kalman Filter implementation
 * 
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <KalmanFilter.h>

namespace astro {
namespace guiding {

KalmanFilter::KalmanFilter(double deltat) : _deltat(deltat) {
	// initialize the phi matrix
	phi(1,1) = 1; phi(1,2) = _deltat;
	phi(2,2) = 1;
	phi(3,3) = 1; phi(3,4) = _deltat;
	phi(4,4) = 1;
	// measurement matrix
	H(0,0) = 1;
	H(1,2) = 1;
	// system error
	// measurement error
}

Point	KalmanFilter::offset() const {
	return Point(x[0], x[1]);
}

void	KalmanFilter::update(const Point& o) {
	// prediction
	Matrix<double,4,4>	Ppred;
	Ppred = phi.transpose() * P * phi + Q;
	Vector<double,4>	xpred;
	xpred = phi * x;

	// measurement
	Vector<double,2>	z;
	z[0] = o.x();
	z[1] = o.y();

	// compute kalman matrix
	K = Ppred * H.transpose() * (H * Ppred * H.transpose() + R).inverse();

	// new state
	Matrix<double,4,4>	I(1);
	x = (I - (K * H)) * xpred + K * z;
	P = (I - (K * H)) * Ppred * (I - (K * H)).inverse() + K * R * K.transpose();
}

} // namespace guiding
} // namespace astro
