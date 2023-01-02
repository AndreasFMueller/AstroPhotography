/*
 * KalmanFilter.cpp -- Kalman Filter implementation
 * 
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <KalmanFilter.h>
#include <AstroDebug.h>

namespace astro {
namespace guiding {

void	KalmanFilter::setup(double deltat) {
	_deltat = deltat;

	// initialize the phi matrix
	phi(0,1) = _deltat;
	phi(2,3) = _deltat;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "system development: %s",
		phi.toString().c_str());

	// system error
	systemerror(systemerror());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "system error: %s",
		Q.toString().c_str());
}

/**
 * \brief Construct the Kalman Filter object
 */
KalmanFilter::KalmanFilter(double deltat) : phi(1) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "initialize KalmanFilter");
	setup(deltat);

	// measurement matrix
	H(0,0) = 1;
	H(1,2) = 1;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "measurement matrix: %s",
		H.toString().c_str());

	// system error
	systemerror(1);

	// measurement error
	measurementerror(1);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "measurement error: %s",
		R.toString().c_str());
}

/**
 * \brief Retrieve the measurement error
 */
double	KalmanFilter::measurementerror() const {
	return sqrt(R(0,0));
}

/**
 * \brief Set the measurement error
 *
 * \param s	variance of the measurement error
 */
void	KalmanFilter::measurementerror(double m) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new measurement error: %.3f", m);
	R(0,0) = m * m;
	R(1,1) = m * m;
}

/**
 * \brief Retrieve the system error
 */
double	KalmanFilter::systemerror() const {
	return sqrt(Q(0,0));
}

/**
 * \brief Set the system error
 *
 * \param s	variance of the system error
 */
void	KalmanFilter::systemerror(double s) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new system error: %.3f", s);
	s = s * s;
	Q(0,0) = s;
	Q(0,1) = s /  _deltat;
	Q(1,0) = s /  _deltat;
	Q(1,1) = s / (_deltat * _deltat);
	Q(2,2) = s;
	Q(2,3) = s /  _deltat;
	Q(3,2) = s /  _deltat;
	Q(3,3) = s / (_deltat * _deltat);
}

/**
 * \brief Retrieve the current filtered offset
 */
Point	KalmanFilter::offset() const {
	Vector<double,4>	xneu = x;
	return Point(xneu[0], xneu[2]);
}

/**
 * \brief Perform the Kalman filter update
 */
void	KalmanFilter::update(const Point& o) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "update with %s", o.toString().c_str());
	// prediction
	Matrix<double,4,4>	Ppred;
	Ppred = phi * P * phi.transpose() + Q;
	Vector<double,4>	xpred;
	xpred = phi * x;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "prediction: %s",
		xpred.toString().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "P prediction: %s",
		Ppred.toString().c_str());

	// measurement
	Vector<double,2>	z;
	z[0] = o.x();
	z[1] = o.y();

	// compute kalman matrix
	K = Ppred * H.transpose() * (H * Ppred * H.transpose() + R).inverse();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new Kalman matrix: %s",
		K.toString().c_str());

	// new state
	Matrix<double,4,4>	I(1);
	x = (I - K * H) * xpred + K * z;
	P = (I - K * H) * Ppred * (I - K * H).transpose() + K * R * K.transpose();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "next state estimate: x = %s",
		x.toString().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "next P estimate: P = %s",
		P.toString().c_str());
}

} // namespace guiding
} // namespace astro
