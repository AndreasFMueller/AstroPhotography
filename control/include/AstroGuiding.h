/*
 * AstroGuiding.h -- classes used to implement guiding
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroGuiding_h
#define _AstroGuiding_h

#include <AstroImage.h>
#include <AstroTypes.h>
#include <AstroAdapter.h>
#include <AstroTransform.h>
#include <AstroCamera.h>
#include <AstroDebug.h>
#include <AstroImager.h>
#include <AstroCallback.h>
#include <AstroLoader.h>
#include <AstroUtils.h>
#include <AstroPersistence.h>
#include <typeinfo>
#include <typeindex>

namespace astro {
namespace guiding {

/**
 * \brief Bad state exception
 *
 * This exception is used to indicate when guider is not in an appropriate
 * state.
 */
class BadState : public std::runtime_error {
public:
	BadState(const std::string& cause) : std::runtime_error(cause) { }
	BadState(const char *cause) : std::runtime_error(cause) { }
};

/**
 * \brief Not found exception
 *
 * This exception is thrown to indicate whether a calibration, tracking
 * history or a device is not available.
 */
class NotFound : public std::runtime_error {
public:
	NotFound(const std::string& cause) : std::runtime_error(cause) { }
	NotFound(const char *cause) : std::runtime_error(cause) { }
};

/**
 * \brief Start Detector base class
 *
 * This is the base class for the star detector. It contains all the relevant
 * functionality that is independent of the Pixel type.
 * This class does not in any way take into account the frame defined
 * in the class. So the coordinates returned in the findResult_s are
 * with respect to the lower left corner of the actual image. This is
 * necessary because an ConstImageAdapter does not have an interface to
 * query the image rectangle.
 */
class StarDetectorBase {
	typedef struct findResult_s {
		ImagePoint	point;
		double	background;
	} findResult;
	findResult	findStar(const image::ConstImageAdapter<double>& _image,
				const ImageRectangle& areaOfInterest) const;
	double	radius(const image::ConstImageAdapter<double>& _image,
				const ImagePoint& where) const;
	Image<RGB<unsigned char> >	*_analysis;
	ImagePtr	_analysis_ptr;
	// auxiliary methods to fill in the analysis image
	void	drawImage(const ConstImageAdapter<double>& image);
	void	drawCentroid(const Point& centroid, double length);
	void	drawTarget(const Point& target, double length);
	void	drawRadius(const ImagePoint& approximage, double radius);
	void	drawHotpixels(const std::list<ImagePoint>& hotpixels);
	void	drawCross(const ImagePoint& point, int length,
			const RGB<unsigned char>& pixel);
	Point	_target;
public:
	Point	target() const { return _target; }
	void	target(const Point& t) { _target = t; }
	StarDetectorBase() { }
	Point	operator()(const image::ConstImageAdapter<double>& _image,
			const image::ImageRectangle& rectangle);
	ImagePtr	analysis() const { return _analysis_ptr; }
};

/**
 * \brief Detector class to determine coordinates if a star
 *
 * Star images are not points, they have a distribution. For guiding,
 * we need to determine the coordinates of the star with subpixel accuracy.
 */
template<typename Pixel>
class StarDetector : public StarDetectorBase {
	const image::ConstImageAdapter<Pixel>&	image;
	adapter::TypeConversionAdapter<Pixel>	tca;
public:
	StarDetector(const image::ConstImageAdapter<Pixel>& _image);
	Point	operator()(const image::ImageRectangle& rectangle);
}; 

/**
 * \brief Create a StarDetector
 */
template<typename Pixel>
StarDetector<Pixel>::StarDetector(
	const image::ConstImageAdapter<Pixel>& _image)
		: image(_image), tca(image) {
}

/**
 * \brief Extract Star coordinates
 *
 * By summing the coordinates weighted by luminance around the maximum pixel
 * value in a rectangle, we get the centroid coordinates of the star's
 * response. This is the best estimate for the star coordinates.
 *
 * We should add a window function here. The problem is that when the
 * image moves, additional stars may get into view. This happens oftion
 * when calibrating. So stars at the border of the image should have much
 * less weight than stars near the center, or stars near the expected
 * position.
 */
template<typename Pixel>
Point	StarDetector<Pixel>::operator()(
		const image::ImageRectangle& rectangle) {
	return StarDetectorBase::operator()(tca, rectangle);
}

Point	findstar(image::ImagePtr image,
	const image::ImageRectangle& rectangle,
	const Point& dither);

/**
 * \brief Tracker class
 *
 * A tracker keeps track off the offset from an initial state. This is the
 * base class that just defines the interface
 */
class Tracker {
protected:
static ConstImageAdapter<double>	*adapter(image::ImagePtr newimage);
public:
	/**
 	 * \brief tracker offset method
	 *
 	 * This is the main method of any tracker. It returns the offset
	 * the tracker has measured.
	 */
	virtual Point	operator()(image::ImagePtr newimage) = 0;
	virtual	std::string	toString() const;
private:
	Point	_dither;
public:
	const Point&	dither() const { return _dither; }
	void	dither(const Point& dither) { _dither = dither; }
protected:
	Point	dithered(const Point& point) { return point + dither(); }
	ImagePtr	_processedImage;
public:
	virtual ImagePtr	processedImage() const {
		return _processedImage;
	}
};

typedef std::shared_ptr<Tracker>	TrackerPtr;

/**
 * \brief A Tracker class that always returns 0 offset
 *
 * This tracker can be used to track blindly, i.e. only applying the
 * constant drift measured in the calibration.
 */
class NullTracker : public Tracker {
public:
	NullTracker() { }
	virtual ~NullTracker() { }
	virtual Point	operator()(image::ImagePtr newimage);
};

/**
 * \brief StarDetector based Tracker
 *
 * This Tracker uses the StarDetector class to track the brightest star
 */
class StarTracker : public Tracker {
	Point	_trackingpoint;
	image::ImageRectangle _searcharea;
	Point	findstar(ImagePtr image, const ImageRectangle& searcharea);
public:
	// constructor
	StarTracker(const Point& point,
		const image::ImageRectangle& searcharea);
	virtual ~StarTracker() { }

	// find the displacement
	virtual Point	operator()(image::ImagePtr newimage);

	// accessors for the tracker configuration data
	const image::ImageRectangle&	searcharea() const {
		return _searcharea;
	}
	image::ImageRectangle&	searcharea() { return _searcharea; }
	void	searcharea(const image::ImageRectangle& r) {
		_searcharea = r;
	}

	const Point&	trackingpoint() const { return _trackingpoint; }
	Point&	trackingpoint() { return _trackingpoint; }
	void	trackingpoint(const Point& p) { _trackingpoint = p; }

	// find a string representation
	virtual std::string	toString() const;
};

std::ostream&	operator<<(std::ostream& out, const StarTracker& tracker);
std::istream&	operator>>(std::ostream& in, StarTracker& tracker);

/**
 * \brief Refreshing functionality for phase correlation tracking
 *
 * Because an image may slightly change over time, the phase correlation
 * tracker becomes worse over time. This can be compensated for by
 * refreshing the first image from time to time. But the new image may
 * have an offset with respect to the original image, so we have to
 * keep track of the offset too, and add it to the new offset.
 */
class RefreshingTracker : public Tracker {
	long	_refreshinterval;
public:
	long	refreshinterval() const { return _refreshinterval; }
	void	refreshinterval(long r) { _refreshinterval = r; }
protected:
	image::ImagePtr	_imageptr;
	Image<double>	*_image;
	double	_lastimagetime;
	Point	_offset;
	bool	refreshNeeded();
	void	refresh(const ConstImageAdapter<double>& adapter,
			const Point offset = Point());
	Point	correlate(const ConstImageAdapter<double>& adapter);
	Point	correlate(const ConstImageAdapter<double>& adapter,
			image::transform::PhaseCorrelator& correlator);
public:
	image::ImagePtr	imageptr() const { return _imageptr; }

	RefreshingTracker();
	virtual ~RefreshingTracker();
	virtual Point	operator()(image::ImagePtr newimage) = 0;
	virtual std::string	toString() const;
};

/**
 * \brief PhaseCorrelator based Tracker
 *
 * This Tracker uses the PhaseCorrelator class. It is to be used in case
 * where there is no good guide star.
 */
template<typename Adapter>
class PhaseTracker : public RefreshingTracker {
public:
	PhaseTracker() { }
	virtual ~PhaseTracker() { }
	virtual Point	operator()(image::ImagePtr newimage) {
		ConstImageAdapter<double>	*a = adapter(newimage);
		if (!_imageptr) {
			Adapter	from(*a);
			refresh(from);
			delete a;
			return Point(0,0);
		}
		Adapter	to(*a);
		Point	result = correlate(to);
		delete a;
		return dithered(result);
	}
};

/**
 * \brief Large Object Tracker
 *
 * This tracker tries to keep a large object in the center of the image
 * by computing the offset of the center of gravity of the objects
 */
class LargeTracker : public Tracker {
public:
	LargeTracker() { }
	virtual ~LargeTracker() { }
	virtual Point	operator()(image::ImagePtr newimage);
};

// Naming of guider, calibration and control devices
//
// At the end, everything depends on an instrument and the guider
// constructed from that instrument. This information is encapsulated
// in a GuiderName. Since a guider name can have multiple control
// devices, ControlDeviceNames are derived from it and have, in addition
// the control type

class ControlDeviceName;
typedef std::shared_ptr<ControlDeviceName>	ControlDeviceNamePtr;

/**
 * \brief GuiderName
 */
class GuiderName {
	// identification of the guider
	std::string	_instrument;
public:
	GuiderName(const std::string& n);
	GuiderName(const GuiderName& other);
	GuiderName&	operator=(const GuiderName& other);

	const std::string&	instrument() const { return _instrument; }

	bool	hasGuidePort() const;
	bool	hasAdaptiveOptics() const;

	ControlDeviceNamePtr	guidePortDeviceName();
	ControlDeviceNamePtr	adaptiveOpticsDeviceName();
};

/**
 * \brief Control device type
 */
typedef enum { GP, AO } ControlDeviceType;
std::string	type2string(ControlDeviceType caltype);
ControlDeviceType	string2type(const std::string& calname);

/**
 * \brief Control device name
 *
 * In addition to the guider, the control device knows which one of the
 * devices in the Guider it is actually controlling, so its name knows
 * the type of device
 */
class	ControlDeviceName : public GuiderName {
	ControlDeviceType	_type;
public:
	ControlDeviceName(const GuiderName& guidername,
		ControlDeviceType type = GP);
	ControlDeviceName(const ControlDeviceName& other);
	ControlDeviceName&	operator=(const ControlDeviceName& other);
	ControlDeviceType	controldevicetype() const;
	void	controldevicetype(ControlDeviceType t);
	void	checktype(ControlDeviceType t);
};

/**
 * \brief CalibrationPoint
 */
class CalibrationPoint {
public:
	double	t;
	Point	offset;	// ra, dec
	Point	star;	// pixel coordinates of observed star
	CalibrationPoint() { }
	CalibrationPoint(double _t, const Point& _offset, const Point& _star)
		: t(_t), offset(_offset), star(_star) { }
	std::string	toString() const;
};

/**
 * \brief Base class for all the calibrations for guiders
 *
 * This is the common ground for calibration of guiders using guider ports
 * and tip-tilt adaptive optics units.
 */
class BasicCalibration : public std::vector<CalibrationPoint> {
	int	_calibrationid;
public:
	int	calibrationid() const { return _calibrationid; }
	void	calibrationid(int c) { _calibrationid = c; }

	// name of the control device for which this calibration was 
	// constructed
private:
	ControlDeviceName	_name;
public:
	const ControlDeviceName&	name() const { return _name; }
	ControlDeviceType	calibrationtype() const {
		return _name.controldevicetype();
	}
	void	calibrationtype(ControlDeviceType ct) {
		_name.controldevicetype(ct);
	}

	// when the calibration was done
private:
	time_t	_when;
public:
	time_t	when() const { return _when; }
	void	when(time_t w) { _when = w; }

	// telescope position in which the calibration was done
private:
	bool	_east;
public:
	bool	east() const { return _east; }
	void	east(bool e) { _east = e; }

private:
	Angle	_declination;
public:
	Angle	declination() const { return _declination; }
	void	declination(const Angle& d) { _declination = d; }

	// calibration coefficients
public:
	double	a[6];
	// access calibration constants adapted for meridian flip
	double	coef(int i) const;
private:
	bool	_complete;
public:
	bool	complete() const { return _complete; }
	void	complete(bool c) { _complete = c; }

	// the _flipped flag can be used if the camera is suddenly upside
	// down, this has nothing to do with meridian flip (see below)
private:
	bool	_flipped;
public:
	int	flippedsign() const { return (_flipped) ? -1 : 1; }
	bool	flipped() const { return _flipped; }
	void	flipped(bool f) { _flipped = f; }
	void	flip() { _flipped = !_flipped; }

	// the _meridian_flipped flag indicates whether the calibration
	// should be used for the telescope after a meridian flip, i.e.
	// usually later in the night. This flag has no effect on the
	// numbers in the a array, but the numbers retrieved with the
	// coef method do take it into account. The functions that compute
	// corrections use the coef methods to retrieve the coefficients.
private:
	bool	_meridian_flipped;
public:
	int	meridian_flipped_sign() const {
		return (_meridian_flipped) ? -1 : 1;
	}
	bool	meridian_flipped() const { return _meridian_flipped; }
	void	meridian_flipped(bool m) { _meridian_flipped = m; }
	void	meridian_flip() { _meridian_flipped = !_meridian_flipped; }

protected:
	void	copy(const BasicCalibration& other);
public:
	BasicCalibration(const ControlDeviceName& name);
	BasicCalibration(const ControlDeviceName& name,
		const double coefficients[6]);
	BasicCalibration(const BasicCalibration& other);
	virtual ~BasicCalibration() { }
	BasicCalibration&	operator=(const BasicCalibration& other);

	double	quality() const;
	double	det() const;
	bool	telescope_east_not_west() const { return det() < 0.; }

private:
	double	_focallength;
public:
	double	focallength() const { return _focallength; }
	void	focallength(double f) { _focallength = f; }

private:
	double	_guiderate;
public:
	double	guiderate() const { return _guiderate; }
	void	guiderate(double g) { _guiderate = g; }

private:
	double	_masPerPixel;
public:
	double	masPerPixel() const { return _masPerPixel; }
	void	masPerPixel(double m) { _masPerPixel = m; }

private:
	double	_interval;
public:
	double	interval() const { return _interval; }
	void	interval(double i) { _interval = i; }

	double	pixel_interval() const;
	double	mas_interval() const;

	// string representation of the baseic 
	std::string	toString() const;

	// corrections
	Point	defaultcorrection() const;
	Point	correction(const Point& offset, double Deltat = 0) const;
	Point	offset(const Point& point, double Deltat = 0) const;

	// modifying the calibration
	void	rescale(double scalefactor);
	bool	iscalibrated() const { return 0. != det(); }

	// add a calibration point
	void	add(const CalibrationPoint& p) { push_back(p); }

	// reset
	void	reset();

	// calibrate
	void	calibrate();
};
typedef std::shared_ptr<BasicCalibration>	CalibrationPtr;

std::ostream&	operator<<(std::ostream& out, const BasicCalibration& cal);
std::istream&	operator>>(std::istream& in, BasicCalibration& cal);

/**
 * \brief GuiderCalibration
 *
 * The Calibration data. The coefficients in the array a correspond to
 * a matrix that describes how the control commands on the guider port
 * translate into displacements of the guider image.
 */
class GuiderCalibration : public BasicCalibration {
public:
	GuiderCalibration(const ControlDeviceName& name);
	GuiderCalibration(const ControlDeviceName& name,
		const double coefficients[6]);
	GuiderCalibration(const BasicCalibration& other);
	GuiderCalibration&	operator=(const BasicCalibration& other);
};

/**
 * \brief class for calibrations of adaptive optics
 */
class AdaptiveOpticsCalibration : public BasicCalibration {
	void	ensuretype();
public:
	AdaptiveOpticsCalibration(const ControlDeviceName& name);
	AdaptiveOpticsCalibration(const ControlDeviceName& name,
		const double coefficients[6]);
	AdaptiveOpticsCalibration(const BasicCalibration& other);
	AdaptiveOpticsCalibration&	operator=(const BasicCalibration& other);
};

/**
 * \brief Encapsulation of the calibration as callback argument
 */
typedef callback::CallbackDataEnvelope<CalibrationPtr>	CalibrationCallbackData;

typedef callback::CallbackDataEnvelope<CalibrationPoint>	CalibrationPointCallbackData;

/**
 * \brief Progress indicator
 */
class ProgressInfo {
public:
	double	t;
	double	progress;
	bool	aborted;
};
typedef callback::CallbackDataEnvelope<ProgressInfo>	ProgressInfoCallbackData;

std::ostream&	operator<<(std::ostream& out, const CalibrationPoint& cal);

/**
 * \brief Class to report data 
 */
class TrackingPoint : public callback::CallbackData {
public:
	double	t;
	Point	trackingoffset;
	Point	correction;
	ControlDeviceType	type;
	TrackingPoint() : t(0) {
		type = GP;
	}
	TrackingPoint(const double& actiontime,
		const Point& offset, const Point& activation)
		: t(actiontime), trackingoffset(offset),
		  correction(activation) {
		type = GP;
	}
	std::string	toString() const;
};

/**
 * \brief The GuiderDescriptor is the key to Guiders in the GuiderFactory
 */
class GuiderDescriptor {
	std::string	_instrument;
	std::string	_ccd;
	std::string	_guideport;
	std::string	_adaptiveoptics;
	void	setup(const std::string& instrumentname);
public:
	GuiderDescriptor(const std::string& instrument,
		const std::string& ccd, const std::string& guideport,
		const std::string& adaptiveoptics)
		: _instrument(instrument), _ccd(ccd),
		  _guideport(guideport), _adaptiveoptics(adaptiveoptics) { }
	GuiderDescriptor(const std::string& instrument);
	void	refresh();
	bool	operator==(const GuiderDescriptor& other) const;
	bool	operator<(const GuiderDescriptor& other) const;
	std::string	instrument() const { return _instrument; }
	std::string	ccd() const { return _ccd; }
	std::string	guideport() const { return _guideport; }
	//void	guideport(const std::string& g) { _guideport = g; }
	std::string	adaptiveoptics() const { return _adaptiveoptics; }
	//void	adaptiveoptics(const std::string& a) {
	//	_adaptiveoptics = a;
	//}
	std::string	toString() const;
};

/**
 * \brief summary information about 
 */
class BasicSummary {
	double	_alpha;
	Point	_average;
	Point	average2;
	int	_count;
public:
	time_t	starttime;
	Point	lastoffset;
	Point	averageoffset() const;
	void	average(const Point& a) { _average = a; }
	Point	variance() const;
	void	variance(const Point& v);
	int	count() const { return _count; }
	void	count(int count) { _count = count; }
	BasicSummary(double alpha = 0.1);
	virtual void	addPoint(const Point& offset);
};

/**
 * \brief Holder class for summary data about tracking
 */
class TrackingSummary : public BasicSummary {
public:
	int	trackingid;
	int	guideportcalid;
	int	adaptiveopticscalid;
	GuiderDescriptor	descriptor;
	TrackingSummary(const std::string& instrument);
	virtual void	addPoint(const Point& offset);
};

// we will need the GuiderProcess class, but as we want to keep the 
// implementation (using low level threads and other nasty things) hidden,
// we only define it in the implementation
class GuiderProcess;
typedef std::shared_ptr<GuiderProcess>	GuiderProcessPtr;
class CalibrationProcess;
typedef std::shared_ptr<CalibrationProcess>	CalibrationProcessPtr;
class BasicProcess;
typedef std::shared_ptr<BasicProcess>	BasicProcessPtr;
class PersistentCalibration;

// declarations for the backlash stuff
typedef enum backlash_e { backlash_dec = 0, backlash_ra = 1 } backlash_t;

/**
 * \brief Backlash raw data point
 */
class BacklashPoint {
public:
	int	id;
	double	time;
	double	xoffset;
	double	yoffset;
	std::string	toString() const;
};
typedef std::vector<BacklashPoint>	BacklashPoints;
typedef callback::CallbackDataEnvelope<BacklashPoint>	CallbackBacklashPoint;
typedef std::shared_ptr<CallbackBacklashPoint>	CallbackBacklashPointPtr;

/**
 * \brief A holder class for the Backlash analysis results
 */
class BacklashResult {
public:
	backlash_t	direction;	// direction
	int	lastpoints;		// how many points to include
	double	interval;		// drive interval
	double	x, y;			// primary direction
	double	longitudinal, lateral;	// errors
	double	forward, backward;	// movements
	double	f, b;			// forward/backward 
	double	offset, drift;
	std::string	toString() const;
	void	clear();
	double	operator()(const int k[4], const BacklashPoint& p);
	BacklashResult() : direction(backlash_dec), interval(0), x(0), y(0),
			   longitudinal(0), lateral(0), forward(0), backward(0),
			   f(0), b(0), offset(0), drift(0) { }
};
typedef callback::CallbackDataEnvelope<BacklashResult>	CallbackBacklashResult;
typedef std::shared_ptr<CallbackBacklashResult>	CallbackBacklashResultPtr;

/**
 * \brief a holder class for backlash data and analysis results
 */
class BacklashData {
public:
	BacklashResult	result;
	BacklashPoints	points;
};
typedef std::shared_ptr<BacklashData>	BacklashDataPtr;

class BacklashWork;
typedef std::shared_ptr<BacklashWork>	BacklashWorkPtr;
typedef astro::thread::Thread<BacklashWork>	BacklashThread;
typedef std::shared_ptr<BacklashThread>	BacklashThreadPtr;

typedef callback::CallbackDataEnvelope<BacklashPoint>	CallbackBacklashPoint;
typedef std::shared_ptr<CallbackBacklashPoint>	CallbackBacklashPointPtr;
typedef callback::CallbackDataEnvelope<BacklashResult>	CallbackBacklashResult;
typedef std::shared_ptr<CallbackBacklashResult>	CallbackBacklashResultPtr;

/**
 * \brief enumeration type for the state of the guider
 */
class Guide {
public:
	typedef enum {
		unconfigured, idle, calibrating, calibrated, guiding,
		darkacquire, flatacquire, imaging, backlash
	} state;
static std::string	state2string(state s);
static state	string2state(const std::string& s);
};

typedef enum { FilterNONE, FilterGAIN, FilterKALMAN } FilterMethod;

/**
 * \brief State machine class for the Guider
 *
 * The state machine ensures that any change is only accepted only if
 * all prerequisites are met.
 */
class GuiderStateMachine {
	Guide::state	_state;
	const char	*statename() const;
public:
	const Guide::state&	state() const { return _state; }
	operator Guide::state () { return _state; }
	operator Guide::state () const { return _state; }

	// construct the state machine
	GuiderStateMachine() : _state(Guide::unconfigured) { }

	// methods to find out whether we can accept a configuration, or
	// start calibration or guiding
	bool	canConfigure() const;
	bool	canStartGuiding() const;
	bool	canStartCalibrating() const;
	bool	canAcceptCalibration() const;
	bool	canFailCalibration() const;
	bool	canStopGuiding() const;
	bool	canStartDarkAcquire() const;
	bool	canEndDarkAcquire() const;
	bool	canStartFlatAcquire() const;
	bool	canEndFlatAcquire() const;
	bool	canStartImaging() const;
	bool	canEndImaging() const;
	bool	canStartBacklash() const;
	bool	canEndBacklash() const;

	// state change methods
	void	configure();
	void	startCalibrating();
	void	addCalibration();
	void	failCalibration();
	void	startGuiding();
	void	stopGuiding();
private:
	Guide::state	_prestate;
public:
	void	startDarkAcquire();
	void	endDarkAcquire();
	void	startFlatAcquire();
	void	endFlatAcquire();
	void	startImaging();
	void	endImaging();
	void	startBacklash();
	void	endBacklash();
};

/**
 * \brief Base class for the guider
 *
 * This class provides everything that the a calibration or guiding process
 * needs, but does not add all the burden of the full guider class, which
 * also is able to controll the various types of processes for calibration
 * and guiding. It is essentially a data holder class.
 * It also includes all the callbacks and methods the send data to them.
 */
class GuiderBase : public GuiderName {
protected:
	GuiderStateMachine	_state;
public:
	virtual Guide::state	state();
	// stuff related to the imager
private:
	camera::Imager	_imager;
public:
	camera::Imager&	imager() { return _imager; }
	camera::CcdPtr	ccd() const { return _imager.ccd(); }
	std::string	ccdname() const { return ccd()->name(); }
	camera::CcdInfo	getCcdInfo() const { return ccd()->getInfo(); }
	int	ccdid() const { return getCcdInfo().getId(); }
	double	pixelsize() const;
	// everything related to exposure
private:
	camera::Exposure	_exposure;
public:
	const camera::Exposure&	exposure() const { return _exposure; }
	camera::Exposure&	exposure() { return _exposure; }
	void	exposure(const camera::Exposure& exposure);

	// We should be able to get images through the imager, using the
	// previously defined exposure structure.
	void	startExposure();
	ImagePtr	getImage();
	void	updateImage(ImagePtr image);
private:
	// remember the most recent image
	ImagePtr	_mostRecentImage;
public:
	image::ImagePtr	mostRecentImage() { return _mostRecentImage; }

	// persistence
private:
	persistence::Database	_database;
public:
	persistence::Database	database() { return _database; }

	// callbacks
private:
	callback::CallbackSet	_imagecallback;
	callback::CallbackSet	_calibrationcallback;
	callback::CallbackSet	_progresscallback;
	callback::CallbackSet	_trackingcallback;
	callback::CallbackSet	_calibrationimagecallback;
	callback::CallbackSet	_backlashcallback;
public:
	void	addImageCallback(callback::CallbackPtr i);
	void	addCalibrationCallback(callback::CallbackPtr c);
	void	addProgressCallback(callback::CallbackPtr c);
	void	addGuidercalibrationCallback(callback::CallbackPtr c);
	void	addTrackingCallback(callback::CallbackPtr t);
	void	addCalibrationImageCallback(callback::CallbackPtr t);
	void	addBacklashCallback(callback::CallbackPtr t);

	void	removeImageCallback(callback::CallbackPtr i);
	void	removeCalibrationCallback(callback::CallbackPtr c);
	void	removeProgressCallback(callback::CallbackPtr c);
	void	removeTrackingCallback(callback::CallbackPtr t);
	void	removeCalibrationImageCallback(callback::CallbackPtr t);
	void	removeBacklashCallback(callback::CallbackPtr t);
	
	void	callback(image::ImagePtr image);
	void	callback(const CalibrationPoint& point);
	void	callback(const ProgressInfo& point);
	void	callback(const CalibrationPtr cal);
	void	callback(const TrackingPoint& point);
	void	callback(const astro::camera::CalibrationImageProgress& calimageprogress);
	void	callback(const BacklashPoint& point);
	void	callback(const BacklashResult& result);
	virtual void	callback(const std::exception& ex) = 0;

	// constructor
	GuiderBase(const GuiderName& guidername, camera::CcdPtr ccd,
		persistence::Database database = NULL);

	// virtual interface
	virtual void	saveCalibration() = 0;
	virtual void	forgetCalibration() = 0;

	// handle the backlash data
protected:
	BacklashData	_backlashdata;
public:
	const BacklashData&	backlashData() const { return _backlashdata; }
};

/**
 * \brief Class handling a control device
 *
 * Control devices are guider ports or adaptive optics units
 */
class ControlDeviceBase {
protected:
	callback::CallbackPtr	_callback;
	// the guider gives access to everything needed from calibration
protected:
	GuiderBase	*_guider;
public:
	const std::string&	instrument() const;
	camera::Imager&		imager();
	std::string	ccdname() const;
	const camera::Exposure&	exposure() const;
	void	exposure(const camera::Exposure& e);

	// persistence and calibration data
protected:
	persistence::Database	_database;
	CalibrationPtr	_calibration;
public:
	CalibrationPtr	calibration() { return _calibration; }
	int	calibrationid() const;
	void	calibrationid(int calid, bool meridian_flipped = false);
	bool	iscalibrated() const;
	bool	flipped() const;
	bool	meridian_flipped() const;
	void	flip();
	void	meridian_flip();

	// parameters about the calibration
private:
	std::map<std::string, double>	parameters;
public:
	bool	hasParameter(const std::string& name) const;
	double	parameter(const std::string& name) const;
	double	parameter(const std::string& name, double value) const;
	void	setParameter(const std::string& name, double value);

	// here comes the device specific stuff, which is of course virtual
	virtual std::string	devicename() const = 0;
	virtual std::type_index	deviceType() const = 0;
	virtual std::type_index	configurationType() const = 0;

	// prevent copying
private:
	ControlDeviceBase(const ControlDeviceBase& other);
	ControlDeviceBase&	operator=(const ControlDeviceBase& other);

	// constructors and destructors
public:
	ControlDeviceBase(GuiderBase *guider,
		persistence::Database database = NULL);
	virtual ~ControlDeviceBase();

	// methods to control calibration
	virtual int	startCalibration(TrackerPtr tracker);
	void	cancelCalibration();
	bool	waitCalibration(double timeout);
	virtual void	saveCalibration();
protected:
	bool	_calibrating;
public:
	bool	calibrating() const { return _calibrating; }
	void	calibrating(bool c) { _calibrating = c; }

protected:
	BasicProcessPtr	process;
public:
	// apply a correction
	virtual Point	correct(const Point& point, double Deltat,
				bool stepping = false);
protected:
	AsynchronousAction	asynchronousaction;
};

typedef std::shared_ptr<ControlDeviceBase>	ControlDevicePtr;

/**
 * \brief template for control devices
 *
 * The template implements different devices: guider ports or adaptive
 * optics units
 */
template<typename device, typename devicecalibration, ControlDeviceType type>
class ControlDevice : public ControlDeviceBase {
public:
	typedef std::shared_ptr<device>	deviceptr;
private:
	deviceptr	_device;
public:
	ControlDevice(GuiderBase *guider, deviceptr dev,
		persistence::Database database = NULL)
		: ControlDeviceBase(guider, database), _device(dev) {
		_calibration = CalibrationPtr(new devicecalibration(
			ControlDeviceName(guider->instrument(), type)));
	}
	virtual ~ControlDevice() {
	}
	virtual std::string	devicename() const { return _device->name(); }
	virtual int	startCalibration(TrackerPtr /* tracker */) {
		return -1; // suppress warning
	}
//	virtual void	calibrationid(int /* calid */) { }
	virtual std::type_index	deviceType() const {
		return typeid(device);
	}
	virtual std::type_index	configurationType() const {
		return typeid(devicecalibration);
	}
	virtual Point	correct(const Point& point, double Deltat,
				bool stepping = false);
};

// specializations for GuidePort
template<>
int	ControlDevice<camera::GuidePort,
		GuiderCalibration, GP>::startCalibration(TrackerPtr tracker);

template<>
Point	ControlDevice<camera::GuidePort,
		GuiderCalibration, GP>::correct(const Point& point, double Deltat,
			bool stepping);

// specializatiobs for adaptive optics
template<>
int	ControlDevice<camera::AdaptiveOptics,
		AdaptiveOpticsCalibration, AO>::startCalibration(TrackerPtr tracker);

template<>
Point	ControlDevice<camera::AdaptiveOptics,
		AdaptiveOpticsCalibration, AO>::correct(const Point& point,
			double Deltat, bool stepping);

/**
 * \brief Guider class
 * 
 * The guider class unifies all the operations needed for guiding.
 * First, it takes care of tracking the state the guider is in, using a
 * GuiderStateMachine object for that purpose. Second, it can handle
 * a process that performs the calibration. This is controlled by the
 * guider calibration commands.
 */
class Guider : public GuiderBase {
	void	checkstate();
public:
	virtual Guide::state	state();
	// The guider is essentially composed of a camera and a guideport
	// we will hardly need access to the camera, but we don't want to
	// loose the reference to it either, so we keep it handy here
private:
	camera::GuidePortPtr	_guideport;
	camera::AdaptiveOpticsPtr	_adaptiveoptics;
public:
	bool	hasGuideport() { return (_guideport) ? true : false; }
	camera::GuidePortPtr	guideport() {
		return _guideport;
	}
	std::string	guideportname() const {
		if (_guideport) {
			return _guideport->name();
		} else {
			return std::string("");
		}
	}

	bool	hasAdaptiveoptics() { return (_adaptiveoptics) ? true : false; }
	camera::AdaptiveOpticsPtr	adaptiveoptics() {
		return _adaptiveoptics;
	}
	std::string	adaptiveopticsname() const {
		if (_adaptiveoptics) {
			return _adaptiveoptics->name();
		} else {
			return std::string("");
		}
	}

	GuiderDescriptor	getDescriptor() const;

private:
	double	_focallength;
public:
	double	focallength() const { return _focallength; }
	void	focallength(double f) { _focallength = f; }

private:
	double	_guiderate;
public:
	double	guiderate() const { return _guiderate; }
	void	guiderate(double g) { _guiderate = g; }

private:

	// prevent copying
	Guider(const Guider& other);
	Guider&	operator=(const Guider& other);
public:
	/**
	 * \brief Construct a guider from camera, ccd, and guideport
	 *
	 * After construction, the Guider only knows about the hardware it can
	 * use for guiding. There is no information about what parts of 
	 * the image to take into consideration when looking for a guide star,
	 * or even how to expose an image.
	 */
	Guider(const GuiderName& guidername,
		camera::CcdPtr ccd, camera::GuidePortPtr guideport,
		camera::AdaptiveOpticsPtr adaptiveoptics,
		persistence::Database database = NULL);
	virtual ~Guider();

	/**
	 * \brief Launch the calibration process
	 *
	 * The calibration process needs information where to lock for the
	 * guide star. This information is encoded in the tracker argument.
	 * The tracker can be asked to analyze an image and to find the
	 * the displacement of a star image that it has found. Most commonly,
	 * a StarTracker object is used for this purpose, which also knows
	 * about the rectangle of the image that was selected and the
	 * coordinates of the star.
	 *
	 * The calibration coefficients depend on the speed of the mount,
	 * the focal length and the pixel size. Currently, the software
	 * only knows about the angular speed for the CGEM mount. 
	 * The focallength allows to compute reasonable
	 * values for the calibration displacements.
	 * \param type 		the type of the device to calibrate
	 * \param tracker	The tracker used for tracking. 
	 * \param gridpixels	number of pixels to dimension the grid
	 * \param east		telescope position
	 * \param declination	the declination of the calibration
	 * \return		the id of the calibration run
	 */
	int	startCalibration(ControlDeviceType type,
			TrackerPtr tracker, float gridpixels,
			bool east = false, Angle declination = Angle());
private:
	void	checkCalibrationState();
public:
	void	saveCalibration();
	void	forgetCalibration();
	void	useCalibration(int calid, bool meridian_flipped = false);
	void	unCalibrate(ControlDeviceType type);

	/**
	 * \brief query the progress of the calibration process
	 */
private:
	double	_progress;
public:
	double	calibrationProgress() { return _progress; }
	void	calibrationProgress(double p);
	/**
	 * \brief cancel the calibratio process
	 */
	void	cancelCalibration();
	/**
	 * \brief wait for the calibration process to complete
	 */
	bool	waitCalibration(double timeout);
private:
	void	calibrationCleanup();

public:
	ControlDevicePtr	guidePortDevice;
	ControlDevicePtr	adaptiveOpticsDevice;

public:
	// methods involved with creating a tracker
	double	getPixelsize();
	TrackerPtr	getTracker(const Point& point);
	TrackerPtr	getNullTracker();
	TrackerPtr	getPhaseTracker();
	TrackerPtr	getDiffPhaseTracker();
	TrackerPtr	getLaplaceTracker();
	TrackerPtr	getLargeTracker();

private:
	BasicProcessPtr	trackingprocess;

public:
	// tracking
	void	startGuiding(TrackerPtr tracker, double interval,
			double aointerval = 0, bool stepping = false,
			FilterMethod filtermethod = FilterNONE);
	void	stopGuiding();
	bool	waitGuiding(double timeout);
	double	getInterval();
	const TrackingSummary&	summary();

	// access to the current tracker, mainly for dithering
	TrackerPtr	currentTracker() const;
	void	dither(const Point& _dither);
	void	ditherArcsec(double arcsec);
	Point	dither() const;
private:
	float	_filter_parameters[2];
public:
	float	filter_parameter(int i);
	void	filter_parameter(int i, float g);
	
public:
	/**
	 * \brief Information about the most recent update
	 *
 	 * The information is returned in the reference arguments
	 * \param actiontime	time when last action occured
	 * \param offset	the tracking offset observed by the tracker
	 * \param activation	the activations computed for the next period
	 */
	void lastAction(double& actiontime, Point& offset, Point& activation);

	void	callback(const std::exception& ex);

	// methods related to getting dark images
private:
	camera::DarkWorkImagerPtr	_darkwork;
	camera::DarkWorkImagerThreadPtr	_darkthread;
public:
	void	startDark(double exposuretime, int imagecount,
			double badpixellimit);
	void	endDark();

	// methods related to getting flat images
private:
	camera::FlatWorkImagerPtr	_flatwork;
	camera::FlatWorkImagerThreadPtr	_flatthread;
public:
	void	startFlat(double exposuretime, int imagecount, bool useDark);
	void	endFlat();

	// acquiring an image
private:
	camera::ImageWorkImagerPtr	_imagework;
	camera::ImageWorkImagerThreadPtr	_imagethread;
	ImagePtr	_imaging;
public:
	void	startImaging(const camera::Exposure& exposure);
	void	endImaging(ImagePtr image);
	ImagePtr	getImaging() { return _imaging; }

	// backlash characterisation 
private:
	BacklashWorkPtr		_backlashwork;
	BacklashThreadPtr	_backlashthread;
public:
	void	startBacklash(TrackerPtr tracker, double interval,
			backlash_t dir);
	void	setLastPoints(int n);
	void	stopBacklash();
	backlash_t	getBacklashDirection();
};
typedef std::shared_ptr<Guider>	GuiderPtr;

/**
 * \brief GuiderFactory class
 */
class GuiderFactory;
typedef std::shared_ptr<GuiderFactory>	GuiderFactoryPtr;
class GuiderFactory {
	module::ModuleRepositoryPtr	repository;
	persistence::Database	database;
	typedef	std::map<GuiderDescriptor, GuiderPtr>	guidermap_t;
	guidermap_t	guiders;
public:
	GuiderFactory() { }
	GuiderFactory(module::ModuleRepositoryPtr _repository,
		persistence::Database _database)
		: repository(_repository), database(_database) { }
	std::vector<GuiderDescriptor>	list() const;
	GuiderPtr	get(const GuiderDescriptor& guiderdescriptor);
	static GuiderFactoryPtr	get();
	static void	initialize(module::ModuleRepositoryPtr _repository,
				persistence::Database _database);
};

/**
 * \brief Encapsulation of the information about a guide run
 */
class Track {
public:
	int	trackid;
	time_t	whenstarted;
	std::string	instrument;
	std::string	ccd;
	std::string	guideport;
	std::string	adaptiveoptics;
	int	guideportcalid;
	int	adaptiveopticscalid;
	Track() { }
	Track(time_t _whenstarted,
		const std::string& _instrument, const std::string& _ccd,
		const std::string& _guideport,
		const std::string& _adaptiveoptics)
		: whenstarted(_whenstarted),
		  instrument(_instrument), ccd(_ccd), guideport(_guideport),
		  adaptiveoptics(_adaptiveoptics) {
		trackid = -1;
		guideportcalid = -1;
		adaptiveopticscalid = -1;
	}
};

/**
 * \brief A class encapsulating a full history, including tracking points
 */
class TrackingHistory : public Track {
public:
	std::list<TrackingPoint>	points;
	TrackingHistory() { }
	TrackingHistory(const Track& track) 
		: Track(track) {
	}
};

/**
 * \brief Calibration class 
 */
class PersistentCalibration {
public:
	time_t	when;
	std::string	instrument;
	std::string	ccd;
	std::string	controldevice;
	int	east;
	double	declination;	// degrees
	double	a[6];
	double	focallength;	// meter
	double	quality;
	double	det;
	int	complete;
	double	masPerPixel;
	int	controltype;
	double	interval;	// seconds
	double	guiderate;	// multiples of the siderial rate
	PersistentCalibration();
	PersistentCalibration(const BasicCalibration& other);
	PersistentCalibration&	operator=(const BasicCalibration& other);
};

typedef persistence::Persistent<PersistentCalibration>	CalibrationRecord;
typedef std::shared_ptr<CalibrationRecord>	CalibrationRecordPtr;

typedef persistence::PersistentRef<CalibrationPoint>	CalibrationPointRecord;

/**
 * \brief a simplified interface to the calibration persistence  tables
 */
class CalibrationStore {
	astro::persistence::Database	_database;
	ControlDeviceName	nameFromRecord(
					const CalibrationRecord& record) const;
public:
	CalibrationStore(astro::persistence::Database database)
		: _database(database) { }
	CalibrationStore();
	std::list<long>	getAllCalibrations();
	std::list<long>	getAllCalibrations(ControlDeviceType);
	std::list<long>	getCalibrations(const GuiderDescriptor& guider,
				ControlDeviceType type);

	// access to calibrations
	bool	contains(long id);
	bool	contains(long id, ControlDeviceType type);
	bool	containscomplete(long id, ControlDeviceType type);
	long	addCalibration(const PersistentCalibration& calibration);
	void	deleteCalibration(long id);
	void	updateCalibration(const CalibrationPtr calibration);

	// guider calibration
	CalibrationPtr	getCalibration(long id);

	// guider points
	std::list<CalibrationPointRecord>	getCalibrationPoints(long id);
	void	addPoint(long id, const CalibrationPoint& point);
	void	removePoints(long id);

	// storing basic calibrations, i.e. just the raw calibration data
	// without all the attributes
	void	saveCalibration(const CalibrationPtr cal);
};

// types used in the tracking store
typedef persistence::Persistent<Track>	TrackRecord;
typedef persistence::PersistentRef<TrackingPoint>	TrackingPointRecord;

/**
 * \brief Simplified interface to tracking history data
 */
class TrackingStore {
	astro::persistence::Database	_database;
public:
	TrackingStore(astro::persistence::Database database)
		: _database(database) { }
	TrackingStore();
	std::list<long>	getAllTrackings();
	std::list<long>	getTrackings(const GuiderDescriptor& guider);
	std::list<TrackingPointRecord>	getHistory(long id);
	std::list<TrackingPointRecord>	getHistory(long id,
		ControlDeviceType type);
	TrackingHistory	get(long id);
	TrackingHistory	get(long id,
		ControlDeviceType type);
	void	deleteTrackingHistory(long id);
	bool	contains(long id);
	TrackingSummary	getSummary(long id);
};

/**
 * \brief Computations for dithering
 */
class DitherCalculator {
	AngularSize	_pixelsize;
public:
	const AngularSize&	pixelsize() const { return _pixelsize; }
	DitherCalculator(const AngularSize& pixelsize);
	Point	ditherArcsec(double arcsec);
	Point	dither(double pixels);
};

} // namespace guiding
} // namespace astro

#endif /* _AstroGuiding_h */
