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
public:
	StarDetectorBase() { }
	Point	operator()(const image::ConstImageAdapter<double>& _image,
			const image::ImageRectangle& rectangle) const;
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
	Point	operator()(const image::ImageRectangle& rectangle) const;
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
		const image::ImageRectangle& rectangle) const {
	return StarDetectorBase::operator()(tca, rectangle);
}

Point	findstar(image::ImagePtr image,
	const image::ImageRectangle& rectangle, int k);

/**
 * \brief Tracker class
 *
 * A tracker keeps track off the offset from an initial state. This is the
 * base class that just defines the interface
 */
class Tracker {
public:
	virtual Point	operator()(image::ImagePtr newimage) = 0;
	virtual	std::string	toString() const;
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
	virtual Point	operator()(image::ImagePtr newimage);
};

/**
 * \brief StarDetector based Tracker
 *
 * This Tracker uses the StarTracker 
 */
class StarTracker : public Tracker {
	Point	_point;
	image::ImageRectangle _rectangle;
	int	_k;
public:
	// constructor
	StarTracker(const Point& point,
		const image::ImageRectangle& rectangle,
		int k);

	// find the displacement
	virtual Point	operator()(image::ImagePtr newimage);

	// accessors for teh tracker configuration data
	const image::ImageRectangle&	rectangle() const {
		return _rectangle;
	}
	image::ImageRectangle&	rectangle() { return _rectangle; }
	void	rectangle(const image::ImageRectangle& r) {
		_rectangle = r;
	}

	const Point&	point() const { return _point; }
	Point&	point() { return _point; }
	void	point(const Point& p) { _point = p; }

	int	k() const { return _k; }
	void	k(const int k) { _k = k; }
	
	// find a string representation
	virtual std::string	toString() const;
};

std::ostream&	operator<<(std::ostream& out, const StarTracker& tracker);
std::istream&	operator>>(std::ostream& in, StarTracker& tracker);

/**
 * \brief Refreshing functionailty for phase correlation tracking
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
	static ConstImageAdapter<double>	*adapter(ImagePtr image);
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
		return result;
	}
};

#if 0
/**
 * \brief Phase correlator based tracker using the differential
 *
 * This tracker uses the phase correlator, but it uses the differential
 * adapter before it does the correlation.
 */
class DifferentialPhaseTracker : public RefreshingTracker {
public:
	DifferentialPhaseTracker();
	virtual Point	operator()(image::ImagePtr newimage);
	virtual std::string	toString() const;
};
#endif

//class GuiderCalibrator;

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
 * This is the common ground for calibraiton of guiders using guider ports
 * and tip-tilt adaptive optics units.
 */
class BasicCalibration : public std::vector<CalibrationPoint> {
	int	_calibrationid;
public:
	int	calibrationid() const { return _calibrationid; }
	void	calibrationid(int c) { _calibrationid = c; }
public:
	typedef enum { GP, AO } CalibrationType;
static std::string	type2string(CalibrationType caltype);
static CalibrationType	string2type(const std::string& calname);
private:
	CalibrationType	_calibrationtype;
public:
	CalibrationType	calibrationtype() const { return _calibrationtype; }
	void	calibrationtype(CalibrationType ct) { _calibrationtype = ct; }

public:
	double	a[6];
private:
	bool	_complete;
public:
	bool	complete() const { return _complete; }
	void	complete(bool c) { _complete = c; }
	BasicCalibration();
	BasicCalibration(const double coefficients[6]);
	virtual ~BasicCalibration() { }
	BasicCalibration&	operator=(const BasicCalibration& other);

	double	quality() const;
	double	det() const;

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
};

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
	double	focallength;
	double	masPerPixel;
	GuiderCalibration();
	GuiderCalibration(const double coefficients[6]);
	GuiderCalibration(const BasicCalibration& other);
	GuiderCalibration&	operator=(const BasicCalibration& other);
};

/**
 * \brief class for calibrations of adaptive optics
 */
class AdaptiveOpticsCalibration : public BasicCalibration {
public:
	AdaptiveOpticsCalibration();
	AdaptiveOpticsCalibration(const double coefficients[6]);
	AdaptiveOpticsCalibration(const BasicCalibration& other);
	AdaptiveOpticsCalibration&	operator=(const BasicCalibration& other);
};

/**
 * \brief Encapsulation of the calibration as callback argument
 */
typedef callback::CallbackDataEnvelope<GuiderCalibration>	GuiderCalibrationCallbackData;

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
 * \brief Calibration Point encapsulation as callback argument
 */
typedef callback::CallbackDataEnvelope<CalibrationPoint>	CalibrationPointCallbackData;

/**
 * \brief BasicCalibrator
 *
 * The BasicCalibrator collects a set of points and computes the calibration
 * data from this. The BasicCalibrator is used by the CalibrationProcess,
 * it adds points during the calibration using the add method, the calibrate
 * method then computes the calibration data.
 */
class BasicCalibrator {
	BasicCalibration	_calibration;
public:
	BasicCalibrator();
	void	add(const CalibrationPoint& calibrationpoint);
	BasicCalibration	calibrate();
};

/**
 * \brief Class to report data 
 */
class TrackingPoint : public callback::CallbackData {
public:
	double	t;
	Point	trackingoffset;
	Point	correction;
	BasicCalibration::CalibrationType	type;
	TrackingPoint() : t(0) {
		type = BasicCalibration::GP;
	}
	TrackingPoint(const double& actiontime,
		const Point& offset, const Point& activation)
		: t(actiontime), trackingoffset(offset),
		  correction(activation) {
		type = BasicCalibration::GP;
	}
	std::string	toString() const;
};

/**
 * \brief The GuiderDescriptor is the key to Guiders in the GuiderFactory
 */
class GuiderDescriptor {
	std::string	_name;
	std::string	_instrument;
	std::string	_ccd;
	std::string	_guiderport;
	std::string	_adaptiveoptics;
public:
	GuiderDescriptor(const std::string& name, const std::string& instrument,
		const std::string& ccd, const std::string& guiderport,
		const std::string& adaptiveoptics)
		: _name(name), _instrument(instrument), _ccd(ccd),
		  _guiderport(guiderport), _adaptiveoptics(adaptiveoptics) { }
	bool	operator==(const GuiderDescriptor& other) const;
	bool	operator<(const GuiderDescriptor& other) const;
	std::string	name() const { return _name; }
	std::string	instrument() const { return _instrument; }
	std::string	ccd() const { return _ccd; }
	std::string	guiderport() const { return _guiderport; }
	void	guiderport(const std::string& g) { _guiderport = g; }
	std::string	adaptiveoptics() const { return _adaptiveoptics; }
	void	adaptiveoptics(const std::string& a) {
		_adaptiveoptics = a;
	}
	std::string	toString() const;
};

/**
 * \brief summary information about 
 */
class BasicSummary {
	double	_alpha;
	Point	_average;
	Point	average2;
public:
	time_t	starttime;
	Point	lastoffset;
	Point	averageoffset() const;
	void	average(const Point& a) { _average = a; }
	Point	variance() const;
	void	variance(const Point& v);
	BasicSummary(double alpha = 0.1);
	void	addPoint(const Point& offset);
};

/**
 * \brief Holder class for summary data about tracking
 */
class TrackingSummary : public BasicSummary {
public:
	int	trackingid;
	int	guiderportcalid;
	int	adaptiveopticscalid;
	GuiderDescriptor	descriptor;
	TrackingSummary(const std::string& name, const std::string& instrument,
		const std::string& ccd, const std::string& guiderport,
		const std::string& adaptiveoptics);
	TrackingSummary(const std::string& name, const std::string& instrument,
		const std::string& ccd);
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

/**
 * \brief Base class for the guider
 *
 * This class provides everything that the a calibration or guiding process
 * needs, but does not add all the burden of the full guider class, which
 * also is able to controll the various types of processes for calibration
 * and guiding. It is essentially a data holder class.
 * It also includes all the callbacks and methods the send data to them.
 */
class GuiderBase {
	// identification of the guider
	std::string	_name;
	std::string	_instrument;
public:
	const std::string&	name() const { return _name; }
	void	name(const std::string& n) { _name = n; }
	const std::string&	instrument() const { return _instrument; }
	void	instrument(const std::string& i) { _instrument = i; }
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
	void	exposure(const camera::Exposure& exposure) {
		_exposure = exposure;
	}

	// We should be able to get images through the imager, using the
	// previously defined exposure structure.
	void	startExposure();
	ImagePtr	getImage();
private:
	// remember the most recent image
	ImagePtr	_mostRecentImage;
public:
	image::ImagePtr	mostRecentImage() { return _mostRecentImage; }

	// persistence
private:
	persistence::Database	_database;
public:
	persistence::Database&	database() { return _database; }

	// callbacks
private:
	callback::CallbackSet	_imagecallback;
	callback::CallbackSet	_calibrationcallback;
	callback::CallbackSet	_progresscallback;
	callback::CallbackSet	_guidercalibrationcallback;
	callback::CallbackSet	_trackingcallback;
public:
	void	addImageCallback(callback::CallbackPtr i);
	void	addCalibrationCallback(callback::CallbackPtr c);
	void	addProgressCallback(callback::CallbackPtr c);
	void	addGuidercalibrationCallback(callback::CallbackPtr c);
	void	addTrackingCallback(callback::CallbackPtr t);

	void	removeImageCallback(callback::CallbackPtr i);
	void	removeCalibrationCallback(callback::CallbackPtr c);
	void	removeProgressCallback(callback::CallbackPtr c);
	void	removeGuidercalibrationCallback(callback::CallbackPtr c);
	void	removeTrackingCallback(callback::CallbackPtr t);
	
	void	callback(image::ImagePtr image);
	void	callback(const CalibrationPoint& point);
	void	callback(const ProgressInfo& point);
	void	callback(const GuiderCalibration& cal);
	void	callback(const TrackingPoint& point);

	// constructor
	GuiderBase(const std::string& instrument, camera::CcdPtr ccd,
		persistence::Database database = NULL);
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
	const std::string&	name() const;
	const std::string&	instrument() const;
	camera::Imager&		imager();
	std::string	ccdname() const;
	const camera::Exposure&	exposure() const;
	void	exposure(const camera::Exposure& e);

	// persistence and calibration data
protected:
	persistence::Database	_database;
	BasicCalibration	*_calibration;
public:
	int	calibrationid() const { return _calibration->calibrationid(); }
	void	calibrationid(int calid);
	bool	iscalibrated() const;

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
	virtual void	saveCalibration(const BasicCalibration& calibration);
	void	addCalibrationPoint(const CalibrationPoint& point);
protected:
	bool	_calibrating;
public:
	bool	calibrating() const { return _calibrating; }
	void	calibrating(bool c) { _calibrating = c; }

protected:
	BasicProcessPtr	process;
public:
	// apply a correction
	virtual Point	correct(const Point& point, double Deltat);
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
template<typename device, typename devicecalibration>
class ControlDevice : public ControlDeviceBase {
public:
	typedef std::shared_ptr<device>	deviceptr;
private:
	deviceptr	_device;
	devicecalibration	_devicecalibration;
public:
	ControlDevice(GuiderBase *guider, deviceptr dev,
		persistence::Database database = NULL)
		: ControlDeviceBase(guider, database), _device(dev) {
		_calibration = &_devicecalibration;
	}
	~ControlDevice() {
	}
	virtual std::string	devicename() const { return _device->name(); }
	virtual int	startCalibration(TrackerPtr /* tracker */) {
		return -1; // suppress warning
	}
	virtual void	saveCalibration(const BasicCalibration& calibration) {
		*_calibration = calibration;
		ControlDeviceBase::saveCalibration(calibration);
	}
	virtual void	calibrationid(int /* calid */) { }
	virtual std::type_index	deviceType() const {
		return typeid(device);
	}
	virtual std::type_index	configurationType() const {
		return typeid(devicecalibration);
	}
	virtual Point	correct(const Point& point, double Deltat);
};

// specializations for GuiderPort
template<>
int	ControlDevice<camera::GuiderPort,
		GuiderCalibration>::startCalibration(TrackerPtr tracker);

template<>
Point	ControlDevice<camera::GuiderPort,
		GuiderCalibration>::correct(const Point& point, double Deltat);

// specializatiobs for adaptive optics
template<>
int	ControlDevice<camera::AdaptiveOptics,
		AdaptiveOpticsCalibration>::startCalibration(TrackerPtr tracker);

template<>
Point	ControlDevice<camera::AdaptiveOptics,
		AdaptiveOpticsCalibration>::correct(const Point& point,
			double Deltat);

/**
 * \brief enumeration type for the state of the guider
 */
class Guide {
public:
	typedef enum {
		unconfigured, idle, calibrating, calibrated, guiding
	} state;
static std::string	state2string(state s);
static state	string2state(const std::string& s);
};

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

	// state change methods
	void	configure();
	void	startCalibrating();
	void	addCalibration();
	void	failCalibration();
	void	startGuiding();
	void	stopGuiding();
};

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
private:
	GuiderStateMachine	_state;
public:
	Guide::state	state() const;
	// The guider is essentially composed of a camera and a guiderport
	// we will hardly need access to the camera, but we don't want to
	// loose the reference to it either, so we keep it handy here
private:
	camera::GuiderPortPtr	_guiderport;
	camera::AdaptiveOpticsPtr	_adaptiveoptics;
public:
	camera::GuiderPortPtr	guiderport() {
		return _guiderport;
	}
	std::string	guiderportname() const {
		return _guiderport->name();
	}

	camera::AdaptiveOpticsPtr	adaptiveoptics() {
		return _adaptiveoptics;
	}
	std::string	adaptiveopticsname() const {
		return _adaptiveoptics->name();
	}

	GuiderDescriptor	getDescriptor() const;

private:
	double	_focallength;
public:
	double	focallength() const { return _focallength; }
	void	focallength(double f) { _focallength = f; }

private:

	// prevent copying
	Guider(const Guider& other);
	Guider&	operator=(const Guider& other);
public:
	/**
	 * \brief Construct a guider from camera, ccd, and guiderport
	 *
	 * After construction, the Guider only knows about the hardware it can
	 * use for guiding. There is no information about what parts of 
	 * the image to take into consideration when looking for a guide star,
	 * or even how to expose an image.
	 */
	Guider(const std::string& instrument,
		camera::CcdPtr ccd, camera::GuiderPortPtr guiderport,
		camera::AdaptiveOpticsPtr adaptiveoptics,
		persistence::Database database = NULL);

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
	 * \return		the id of the calibration run
	 */
	int	startCalibration(BasicCalibration::CalibrationType type,
			TrackerPtr tracker);
	void	saveCalibration(const GuiderCalibration& calibration);
	void	forgetCalibration();
	void	useCalibration(int calid);
	void	unCalibrate(BasicCalibration::CalibrationType type);

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
	ControlDevicePtr	guiderPortDevice;
	ControlDevicePtr	adaptiveOpticsDevice;

public:
	// methods involved with creating a tracker
	double	getPixelsize();
	TrackerPtr	getTracker(const Point& point);
	TrackerPtr	getNullTracker();
	TrackerPtr	getPhaseTracker();
	TrackerPtr	getDiffPhaseTracker();
	TrackerPtr	getLaplaceTracker();

private:
	BasicProcessPtr	trackingprocess;

public:
	// tracking
	void	startGuiding(TrackerPtr tracker, double interval,
			double aointerval = 0);
	void	stopGuiding();
	bool	waitGuiding(double timeout);
	double	getInterval();
	const TrackingSummary&	summary();
	
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
};
typedef std::shared_ptr<Guider>	GuiderPtr;

/**
 * \brief GuiderFactory class
 */
class GuiderFactory {
	module::Repository	repository;
	persistence::Database	database;
	typedef	std::map<GuiderDescriptor, GuiderPtr>	guidermap_t;
	guidermap_t	guiders;
public:
	GuiderFactory() { }
	GuiderFactory(module::Repository _repository,
		persistence::Database& _database)
		: repository(_repository), database(_database) { }
	std::vector<GuiderDescriptor>	list() const;
	GuiderPtr	get(const GuiderDescriptor& guiderdescriptor);
};
typedef std::shared_ptr<GuiderFactory>	GuiderFactoryPtr;

} // namespace guiding
} // namespace astro

#endif /* _AstroGuiding_h */
