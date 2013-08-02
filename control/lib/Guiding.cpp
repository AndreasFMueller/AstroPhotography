/*
 * Guiding.cpp -- classes implementing guiding
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <AstroIO.h>
#include <GuiderProcess.h>
#include <Accelerate/Accelerate.h>
#include <includes.h>
#include <AstroFormat.h>
#include <AstroCallback.h>

using namespace astro::image;
using namespace astro::image::transform;
using namespace astro::camera;
using namespace astro::guiding;
using namespace astro::io;
using namespace astro::callback;

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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new point: %s",
		newpoint.toString().c_str());
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
// Callback for images retrieved (to help analysis of guider problems)
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Guider Calibration data
//////////////////////////////////////////////////////////////////////
std::string	GuiderCalibration::toString() const {
	return stringprintf("[ %.3f, %.3f, %.3f; %.3f, %.3f, %.3f ]",
		a[0], a[1], a[2], a[3], a[4], a[5]);
}

/**
 * \brief compute correction for drift
 * 
 * While a correction for some offset depends on the time within which
 * the correction should be done, 
 */
Point	GuiderCalibration::defaultcorrection() const {
	return this->operator()(Point(0, 0), 1);
}

/**
 * \brief Compute correction for an offset
 *
 * The correction to be applied to right ascension and declination depends
 * on the time allotted to the correction. The result is a pair of total
 * corrections. They can either be applied in one second, without any
 * corrections in the remaining seconds of the Deltat-interval, or they can
 * be distributed over the seconds of the Deltat-interval.  This distribution,
 * however, has to be calculated by the caller.
 */
Point	GuiderCalibration::operator()(const Point& offset, double Deltat) const {
	double	Deltax = offset.x() + Deltat * a[2];
	double	Deltay = offset.y() + Deltat * a[5];
        double	determinant = a[0] * a[4] - a[3] * a[1];
        double	x = (Deltax * a[4] - Deltay * a[1]) / determinant;
        double	y = (a[0] * Deltay - a[3] * Deltax) / determinant;
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
		A[i        ] = ci->offset.x();
		A[i +     m] = ci->offset.y();
		A[i + 2 * m] = ci->t;
		A[i + 3 * m] = 0;
		A[i + 4 * m] = 0;
		A[i + 5 * m] = 0;
		A[i + 6 * m] = 1;
		A[i + 7 * m] = 0;

		b[i] = ci->point.x();

		i++;

		A[i        ] = 0;
		A[i +     m] = 0;
		A[i + 2 * m] = 0;
		A[i + 3 * m] = ci->offset.x();
		A[i + 4 * m] = ci->offset.y();
		A[i + 5 * m] = ci->t;
		A[i + 6 * m] = 0;
		A[i + 7 * m] = 1;

		b[i] = ci->point.y();

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

Guider::Guider(GuiderPortPtr _guiderport, Imager _imager)
	: guiderport(_guiderport), imager(_imager) {
	calibrated = false;
	// default exposure settings
	exposure.exposuretime = 1.;
	gridconstant = 10;
}

const Exposure&	Guider::getExposure() const {
	return exposure;
}

void	Guider::setExposure(const Exposure& _exposure) {
	exposure = _exposure;
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
 * \param focallength    focallength of guide scope in mm
 * \param pixelsize      size of pixels in um
 */
bool	Guider::calibrate(TrackerPtr tracker,
		double focallength, double pixelsize) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start calibrating");

	// grid range we want to scan
	int range = 1;

	// the grid constant normally depends on the focallength and the
	// pixels size. Smaller pixels are larger focallength allow to
	// use a smaller grid constant. The default value of 10 is a good
	// choice for a 100mm guide scope and 7u pixels as for the SBIG
	// ST-i guider kit
	if ((focallength > 0) && (pixelsize > 0)) {
		gridconstant = 10 * (pixelsize / 7.4) / (focallength / 100);
		if (gridconstant < 2) {
			gridconstant = 2;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using grid constant %f",
			gridconstant);
	}

	// prepare a GuiderCalibrator class that does the actual computation
	GuiderCalibrator	calibrator;

	// perform a grid search
	for (int ra = -range; ra <= range; ra++) {
		for (int dec = -range; dec <= range; dec++) {
			// move the telescope to the grid position
			moveto(gridconstant * ra, gridconstant * dec);
			imager.startExposure(exposure);
			ImagePtr	image = getImage();
			Point	point = (*tracker)(image);
			double	t = now();
			calibrator.add(t, Point(ra, dec), point);
			// move the telescope back
			moveto(-gridconstant * ra, -gridconstant * dec);
			imager.startExposure(exposure);
			image = getImage();
			point = (*tracker)(image);
			t = now();
			calibrator.add(t, Point(0, 0), point);
		}
	}
	
	// now compute the calibration data
	calibration = calibrator.calibrate();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration: %s",
		calibration.toString().c_str());
	calibrated = true;

	// fix time constant
	calibration.a[0] /= gridconstant;
	calibration.a[1] /= gridconstant;
	calibration.a[3] /= gridconstant;
	calibration.a[4] /= gridconstant;

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
void	Guider::moveto(double ra, double dec) {
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
	if (raplus > t) {
		t = raplus;
	}
	if (raminus > t) {
		t = raminus;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "RA: raplus = %f, raminus = %f",
		raplus, raminus);
	guiderport->activate(raplus, raminus, 0, 0);
	sleep(t);

	t = 0;
	if (dec > 0) {
		decplus = dec;
	} else {
		decminus = -dec;
	}
	if (decminus > t) {
		t = decminus;
	}
	if (decplus > t) {
		t = decplus;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "DEC: decplus = %f, decminus = %f",
		decplus, decminus);
	guiderport->activate(0, 0, decplus, decminus);
	sleep(t);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "moveto complete");
}

/**
 * \brief start an exposure
 */
void	Guider::startExposure() {
	imager.startExposure(exposure);
}

/**
 * \brief get the image
 */
ImagePtr	Guider::getImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getImage() called");
	ImagePtr	image = imager.getImage();
	if (newimagecallback) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "sending new image to callback");
		GuiderNewImageCallbackData	*argp = 
			new GuiderNewImageCallbackData(image);
		CallbackDataPtr	arg(argp);
		newimagecallback->operator()(arg);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "callback return");
	}
	return image;
}

/**
 * \brief Utility function: pause for a number of seconds
 *
 * \param t	time in seconds. 
 */
void	Guider::sleep(double t) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sleep for %.3f seconds", t);
	unsigned int	tt = 1000000 * t;
	usleep(tt);
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

Imager	Guider::getImager() {
	return imager;
}

/**
 * \brief Accessor to calibrationd ata
 */
const GuiderCalibration&	Guider::getCalibration() const {
	return calibration;
}

} // namespace guiding
} // namespace astro
