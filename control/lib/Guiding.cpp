/*
 * Guiding.cpp -- classes implementing guiding
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <GuiderProcess.h>
#include <Accelerate/Accelerate.h>
#include <includes.h>

using namespace astro::image;
using namespace astro::image::transform;
using namespace astro::camera;
using namespace astro::guiding;

namespace astro {
namespace guiding {

#define	findstar_typed(Pixel)						\
{									\
	Image<Pixel >	*imagep						\
		= dynamic_cast<Image<Pixel > *>(&*image);		\
	if (NULL != imagep) {						\
		StarDetector<Pixel >	sd(*imagep);			\
		return sd(rectangle, k);				\
	}								\
}

Point	findstar(ImagePtr image, const ImageRectangle& rectangle,
		unsigned int k) {
	findstar_typed(unsigned char);
	findstar_typed(unsigned short);
	findstar_typed(unsigned int);
	findstar_typed(unsigned long);
	findstar_typed(float);
	findstar_typed(double);
	findstar_typed(RGB<unsigned char>);
	findstar_typed(RGB<unsigned short>);
	findstar_typed(RGB<unsigned int>);
	findstar_typed(RGB<unsigned long>);
	findstar_typed(RGB<float>);
	findstar_typed(RGB<double>);
	findstar_typed(YUYV<unsigned char>);
	findstar_typed(YUYV<unsigned short>);
	findstar_typed(YUYV<unsigned int>);
	findstar_typed(YUYV<unsigned long>);
	findstar_typed(YUYV<float>);
	findstar_typed(YUYV<double>);
	throw std::runtime_error("cannot find star in this image type");
}

StarTracker::StarTracker(const Point& _point,
	const ImageRectangle& _rectangle, unsigned int _k)
	: point(_point), rectangle(_rectangle), k(_k) {
}

Point	StarTracker::operator()(ImagePtr newimage)
	const {
	// find the star on the new image
	Point	newpoint = findstar(newimage, rectangle, k);
	return newpoint - point;
}

const ImageRectangle&	StarTracker::getRectangle() const {
	return rectangle;
}

#define	phasetracker_construct(Pixel)					\
{									\
	Image<Pixel>	*imagep						\
		= dynamic_cast<Image<Pixel > *>(&*_image);		\
	if (NULL != imagep) {						\
		LuminanceAdapter<Pixel >	la(*imagep);		\
		image = ImagePtr(new Image<double>(la));		\
		return;							\
	}								\
}

PhaseTracker::PhaseTracker(ImagePtr _image) {
	phasetracker_construct(unsigned char);
	phasetracker_construct(unsigned short);
	phasetracker_construct(unsigned int);
	phasetracker_construct(unsigned long);
	phasetracker_construct(float);
	phasetracker_construct(double);
	phasetracker_construct(RGB<unsigned char>);
	phasetracker_construct(RGB<unsigned short>);
	phasetracker_construct(RGB<unsigned int>);
	phasetracker_construct(RGB<unsigned long>);
	phasetracker_construct(RGB<float>);
	phasetracker_construct(RGB<double>);
	phasetracker_construct(YUYV<unsigned char>);
	phasetracker_construct(YUYV<unsigned short>);
	phasetracker_construct(YUYV<unsigned int>);
	phasetracker_construct(YUYV<unsigned long>);
	phasetracker_construct(YUYV<float>);
	phasetracker_construct(YUYV<double>);
	throw std::runtime_error("cannot track this image type");
}

#define	phasetracker_typed(Pixel)					\
{									\
	Image<Pixel>	*newimagep					\
		= dynamic_cast<Image<Pixel > *>(&*newimage);		\
	if (NULL != newimagep) {					\
		LuminanceAdapter<Pixel >	newla(*newimagep);	\
		Image<double>	*imagep					\
			= dynamic_cast<Image<double> *>(&*image);	\
		PhaseCorrelator	pc;					\
		return pc(*imagep, newla);				\
	}								\
}

Point	PhaseTracker::operator()(ImagePtr newimage)
	const {
	phasetracker_typed(unsigned char);
	phasetracker_typed(unsigned short);
	phasetracker_typed(unsigned int);
	phasetracker_typed(unsigned long);
	phasetracker_typed(float);
	phasetracker_typed(double);
	phasetracker_typed(RGB<unsigned char>);
	phasetracker_typed(RGB<unsigned short>);
	phasetracker_typed(RGB<unsigned int>);
	phasetracker_typed(RGB<unsigned long>);
	phasetracker_typed(RGB<float>);
	phasetracker_typed(RGB<double>);
	phasetracker_typed(YUYV<unsigned char>);
	phasetracker_typed(YUYV<unsigned short>);
	phasetracker_typed(YUYV<unsigned int>);
	phasetracker_typed(YUYV<unsigned long>);
	phasetracker_typed(YUYV<float>);
	phasetracker_typed(YUYV<double>);
	throw std::runtime_error("cannot track this image type");
}

//////////////////////////////////////////////////////////////////////
// Guider Calibration data
//////////////////////////////////////////////////////////////////////
std::string	GuiderCalibration::toString() const {
	return stringprintf("[ %.3f, %.3f, %.3f; %.3f, %.3f, %.3f ]",
		a[0], a[1], a[2], a[3], a[4], a[5]);
}

Point	GuiderCalibration::defaultcorrection() const {
	return this->operator()(Point(a[2], a[5]));
}

Point	GuiderCalibration::operator()(const Point& offset) const {
        double determinant = a[0] * a[4] - a[1] * a[3];
        double	x = (offset.x * a[4] - offset.y * a[3]) / determinant;
        double	y = (a[0] * offset.x - a[1] * offset.y) / determinant;
	Point	result(x, y);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "correction for offset %s: %s",
		offset.toString().c_str(), result.toString().c_str());
	return result;
}

//////////////////////////////////////////////////////////////////////
// Guider Calibrator
//////////////////////////////////////////////////////////////////////
GuiderCalibrator::GuiderCalibrator() {
}

void	GuiderCalibrator::add(double t, const Point& offset,
		const Point& point) {
	calibration_data.push_back(calibration_point(t, offset, point));
}

GuiderCalibration	GuiderCalibrator::calibrate() {
	// build the linear system of equations
	int	m = 2 * calibration_data.size(); // number of equations
	int	n = 8; // number of unknowns
	double	A[n * m];
	double	b[m];

	// fill in equations
	std::vector<calibration_point>::const_iterator	ci;
	int	i = 0;
	for (ci = calibration_data.begin(); ci != calibration_data.end(); ci++){
		A[i        ] = ci->offset.x;
		A[i +     m] = ci->offset.y;
		A[i + 2 * m] = ci->t;
		A[i + 3 * m] = 0;
		A[i + 4 * m] = 0;
		A[i + 5 * m] = 0;
		A[i + 6 * m] = 1;
		A[i + 7 * m] = 0;

		b[i] = ci->point.x;

		i++;

		A[i        ] = 0;
		A[i +     m] = 0;
		A[i + 2 * m] = 0;
		A[i + 3 * m] = ci->offset.x;
		A[i + 4 * m] = ci->offset.y;
		A[i + 5 * m] = ci->t;
		A[i + 6 * m] = 0;
		A[i + 7 * m] = 1;

		b[i] = ci->point.y;

		i++;
	}

	// prepare to solve the system using LAPACK (dgels_)
	char	trans = 'N';
	int	nrhs = 1;
	int	lda = m;
	int	ldb = m;
	int	lwork = -1;
	int	info = 0;

	// determine work area size
	double	x;
	dgels_(&trans, &m ,&n, &nrhs, A, &lda, b, &ldb, &x, &lwork, &info);
	if (info != 0) {
		std::string	msg = stringprintf("dgels cannot determine "
			"work area size: %d", info);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	lwork = x;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "need work area of size %d", lwork);

	//  allocate work array
	double	work[lwork];
	dgels_(&trans, &m ,&n, &nrhs, A, &lda, b, &ldb, work, &lwork, &info);
	if (info != 0) {
		std::string	msg = stringprintf("dgels cannot solve "
			"equations: %d", info);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// store the results in the calibration data array
	GuiderCalibration	calibration;
	for (unsigned int i = 0; i < 6; i++) {
		calibration.a[i] = b[i];
	}

	// return the calibration data
	return calibration;
}


//////////////////////////////////////////////////////////////////////
// Guider implementation
//////////////////////////////////////////////////////////////////////

static double	now() {
	struct timeval	tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + 0.000001 * tv.tv_usec;
}

Guider::Guider(GuiderPortPtr _guiderport, CcdPtr _ccd)
	: guiderport(_guiderport), ccd(_ccd) {
	calibrated = false;
}

/**
 * \brief Calibrate the guiding system
 *
 * This method assumes that the observed star position depends linearly
 * on time and the applied correction. It then performs several position
 * measurements and solves for the equation. The resulting matrix should have
 * two nearly perpendicular columns.
 *
 * The mesurements are placed in a grid pattern with coordinate (ra, dec)
 * corresponding to a point that can be reached from the initial position
 * by speeing up (down for negative values) the right ascension/declination
 * motors for ra resp. dec seconds. After each measurement, we return to the
 * central position.
 *
 * This method may require additional parameters to be completely useful.
 */
bool	Guider::calibrate(TrackerPtr tracker) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start calibrating");

	// prepare Exposure structure (XXX we should be able to define the
	// image rectangle an the exposure time)
	Exposure	exposure(ccd->getInfo().getFrame(), 1.);

	// grid range we want to scan
	int range = 1;

	// prepare a GuiderCalibrator class that does the actual computation
	GuiderCalibrator	calibrator;

	// perform a grid search
	for (int ra = -range; ra <= range; ra++) {
		for (int dec = -range; dec <= range; dec++) {
			// move the telescope to the grid position
			moveto(ra, dec);
			ccd->startExposure(exposure);
			Point	point = (*tracker)(ccd->getImage());
			double	t = now();
			calibrator.add(t, Point(ra, dec), point);
			// move the telescope back
			moveto(-ra, -dec);
			ccd->startExposure(exposure);
			point = (*tracker)(ccd->getImage());
			t = now();
			calibrator.add(t, Point(0, 0), point);
		}
	}
	
	// now compute the calibration data
	calibration = calibrator.calibrate();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration: %s",
		calibration.toString().c_str());
	calibrated = true;

	// are we now calibrated?
	return calibrated;
}

/**
 * \brief Move to a grid position
 *
 * Given grid position (ra,dec), move the telescope to this grid position,
 * by actuating right ascension and declination guider ports for the 
 * corresponding number of seconds.
 */
void	Guider::moveto(int ra, int dec) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "moveto (%d, %d)", ra, dec);
	double	t = 0;
	double	raplus = 0;
	double	raminus = 0;
	double	decplus = 0;
	double	decminus = 0;
	if (ra > 0) {
		raplus = ra;
	} else {
		raminus = -ra;
	}
	if (dec > 0) {
		decplus = dec;
	} else {
		decminus = -dec;
	}
	if (raplus > t) {
		t = raplus;
	}
	if (raminus > t) {
		t = raminus;
	}
	if (decminus > t) {
		t = decminus;
	}
	if (decplus > t) {
		t = decplus;
	}
	guiderport->activate(raplus, raminus, decplus, decminus);
	sleep(t);
}

/**
 * \brief Utility function: pause for a number of seconds
 *
 * \param t	time in seconds. Actual precision depends on the resolution
 *		of the systems select call.
 */
void	Guider::sleep(double t) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sleep for %.3f seconds", t);
	struct timeval	tv;
	tv.tv_sec = trunc(t);
	tv.tv_usec = trunc(1000000 * (t - tv.tv_sec));
	if (tv.tv_usec < 0) {
		tv.tv_usec = 0;
	}
	select(0, NULL, NULL, NULL, &tv);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sleep complete");
}

/**
 * \brief start tracking
 */
bool	Guider::start(TrackerPtr tracker) {
	// create a GuiderProcess instance
	guiderprocess = GuiderProcessPtr(new GuiderProcess(*this));
	guiderprocess->start(tracker);
	return true;
}

bool	Guider::stop() {
	guiderprocess->stop();
	return true;
}

GuiderPortPtr	Guider::getGuiderPort() {
	return guiderport;
}

CcdPtr	Guider::getCcd() {
	return ccd;
}

/**
 * \brief Accessor to calibrationd ata
 */
const GuiderCalibration&	Guider::getCalibration() const {
	return calibration;
}

} // namespace guiding
} // namespace astro
