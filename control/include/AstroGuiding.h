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

namespace astro {
namespace guiding {

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
	virtual	std::string	toString() const = 0;
};

typedef std::shared_ptr<Tracker>	TrackerPtr;

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
 * \brief PhaseCorrelator based Tracker
 *
 * This Tracker uses the PhaseCorrelator class. It is to be used in case
 * where there is no good guide star.
 */
class PhaseTracker : public Tracker {
	image::ImagePtr	imageptr;
	Image<double>	*image;
public:
	PhaseTracker();
	virtual Point	operator()(image::ImagePtr newimage);
	virtual std::string	toString() const;
};

/**
 * \brief Phase correlator based tracker using the differential
 *
 * This tracker uses the phase correlator, but it uses the differential
 * adapter before it does the correlation.
 */
class DifferentialPhaseTracker : public Tracker {
	image::ImagePtr	imageptr;
	Image<double>	*image;
public:
	DifferentialPhaseTracker();
	virtual Point	operator()(image::ImagePtr newimage);
	virtual std::string	toString() const;
};

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
};

/**
 * \brief Base class for all the calibrations for guiders
 *
 * This is the common ground for calibraiton of guiders using guider ports
 * and tip-tilt adaptive optics units.
 */
class BasicCalibration : public std::vector<CalibrationPoint> {
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

	double	quality() const;
	double	det() const;

	// string representation of the baseic 
	std::string	toString() const;

	// corrections
	Point	defaultcorrection() const;
	Point	operator()(const Point& offset, double Deltat) const;

	// modifying the calibration
	void	rescale(double scalefactor);
	bool	iscalibrated() const { return 0. != det(); }

	// add a calibration point
	void	add(const CalibrationPoint& p) { push_back(p); }
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
	TrackingPoint() : t(0) { }
	TrackingPoint(const double& actiontime,
		const Point& offset, const Point& activation)
		: t(actiontime), trackingoffset(offset),
		  correction(activation) {
	}
	std::string	toString() const;
};

/**
 * \brief The GuiderDescriptor is the key to Guiders in the GuiderFactory
 */
class GuiderDescriptor {
	std::string	_instrument;
	std::string	_ccd;
	std::string	_guiderport;
public:
	GuiderDescriptor(const std::string& instrument,
		const std::string& ccd,
		const std::string& guiderport)
		: _instrument(instrument), _ccd(ccd),
		  _guiderport(guiderport) { }
	bool	operator==(const GuiderDescriptor& other) const;
	bool	operator<(const GuiderDescriptor& other) const;
	std::string	instrument() const { return _instrument; }
	std::string	ccd() const { return _ccd; }
	std::string	guiderport() const { return _guiderport; }
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
	int	calibrationid;
	GuiderDescriptor	descriptor;
	TrackingSummary(const std::string& instrument,
		const std::string& ccd, const std::string& guiderport);
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
 * \brief Class handling a control device
 *
 * Control devices are guider ports or adaptive optics units
 */
class ControlDeviceBase {
	std::string		_instrument;
	camera::Imager&		_imager;
	int			_calibrationid;
	camera::Exposure	_exposure;
	double			_focallength;
protected:
	persistence::Database	_database;
	PersistentCalibration	*pcal;
public:
	const std::string&	instrument() const { return _instrument; }
	camera::Imager&		imager() { return _imager; }
	std::string	ccdname() const { return _imager.ccd()->name(); }
	virtual std::string	devicename() const = 0;
	int	calibrationid() const { return _calibrationid; }
	const camera::Exposure&	exposure() const { return _exposure; }
	void	exposure(const camera::Exposure& e) { _exposure = e; }
	double	focallength() const { return _focallength; }
	void	focallength(double f) { _focallength = f; }
private:
	ControlDeviceBase(const ControlDeviceBase& other);
	ControlDeviceBase&	operator=(const ControlDeviceBase& other);
public:
	ControlDeviceBase(const std::string& instrument, camera::Imager& imager,
		persistence::Database database = NULL);
	virtual ~ControlDeviceBase();

	virtual int	startCalibration(TrackerPtr tracker);
	void	cancelCalibration();
	bool	waitCalibration(double timeout);
	virtual void	saveCalibration(const BasicCalibration& calibration);
protected:
	BasicProcessPtr	process;
public:

};

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
	devicecalibration	_calibration;
public:
	ControlDevice(const std::string& instrument, camera::Imager& imager,
		deviceptr dev, persistence::Database database = NULL)
		: ControlDeviceBase(instrument, imager, database),
		  _device(dev) {
	}
	virtual std::string	devicename() const { return _device->name(); }
	virtual int	startCalibration(TrackerPtr /* tracker */) {
		return -1; // suppress warning
	}
	virtual void	saveCalibration(const BasicCalibration& calibration) {
		_calibration = calibration;
		ControlDeviceBase::saveCalibration(calibration);
	}
};

// specializations
template<>
int	ControlDevice<camera::GuiderPort, GuiderCalibration>::startCalibration(
		TrackerPtr tracker);
template<>
int	ControlDevice<camera::AdaptiveOptics, AdaptiveOpticsCalibration>::startCalibration(
		TrackerPtr tracker);

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
class Guider {
	void	checkstate();
private:
	GuiderStateMachine	_state;
public:
	Guide::state	state() const;
	// The guider is essentially composed of a camera and a guiderport
	// we will hardly need access to the camera, but we don't want to
	// loose the reference to it either, so we keep it handy here
private:
	std::string	_instrument;
	camera::GuiderPortPtr	_guiderport;
public:
	const std::string&	instrument() const { return _instrument; }
	void	instrument(const std::string& i) { _instrument = i; }
	camera::GuiderPortPtr	guiderport() { return _guiderport; }
	std::string	guiderportname() const { return _guiderport->name().toString(); }
private:
	/*
	 * \brief Image for guiding
	 *
	 * Imaging is performed via the imager, but the imager is built from
	 * from the ccd and the some additional data like darks and flats.
	 * The imager is exposed so that the client can change the imager
	 * parameters, e. g. can add a dark image, or enable pixel
	 * interpolation
	 */
	camera::Imager	_imager;
public:
	const camera::Imager&	imager() const { return _imager; }
	camera::Imager&	imager() { return _imager; }
	camera::CcdPtr	ccd() const { return _imager.ccd(); }
	std::string	ccdname() const { return ccd()->name().toString(); }
	camera::CcdInfo	getCcdInfo() const { return _imager.ccd()->getInfo(); }
	int	ccdid() const { return getCcdInfo().getId(); }

	GuiderDescriptor	getDescriptor() const;

private:
	double	_focallength;
public:
	double	focallength() const { return _focallength; }
	void	focallength(double f) { _focallength = f; }

	/**
	 * \brief Exposure information for guiding images
 	 *
	 * Controlling the exposure parameters includes changing the rectangle
	 * to use during exposure. Since we don't want to implement methods
	 * for all these details, we just expose the exposure structure
	 */
private:
	camera::Exposure	_exposure;
public:
	const camera::Exposure&	exposure() const { return _exposure; }
	camera::Exposure&	exposure() { return _exposure; }
	void	exposure(const camera::Exposure& exposure) {
		_exposure = exposure;
	}
private:
	persistence::Database	_database;

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
		persistence::Database database = NULL);

	// We should be able to get images through the imager, using the
	// previously defined exposure structure.
public:
	void	startExposure();
	ImagePtr	getImage();

	/**
	 * \brief Calibration of the guider
 	 *
	 * Guiding is only possible after the Guider has been calibrated.
	 * The calibration information is stored in the _calibration
	 * member variable, and is accessible through a set of accessors.
	 */
private:
	GuiderCalibration	_calibration;
public:
	const GuiderCalibration&	calibration() const { return _calibration; }
	GuiderCalibration&	calibration() { return _calibration; }
	void	calibration(const GuiderCalibration& calibration);
	bool	iscalibrated() const { return _calibration.iscalibrated(); }

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
	 * \param tracker	The tracker used for tracking. 
	 * \param focallength	Focallength of the optics used for guiding,
	 *			in m.
	 * \return		the id of the calibration run
	 */
	int	startCalibration(TrackerPtr tracker);
	void	saveCalibration(const GuiderCalibration& calibration);

	/**
	 * \brief query the progress of the calibration process
	 */
	double	calibrationProgress();
	/**
	 * \brief cancel the calibratio process
	 */
	void	cancelCalibration();
	/**
	 * \brief wait for the calibration process to complete
	 */
	bool	waitCalibration(double timeout);
	int	calibrationid() { return _calibrationid; }
	void	calibrationid(int calid) { _calibrationid = calid; }
private:
	int	_calibrationid;
	CalibrationProcessPtr	calibrationprocess;
	friend class CalibrationProcess;
	void	calibrationCleanup();

public:
	/**
	 * \brief calibration update callback
	 *
	 * This callback is called with a CalibrationPointCallbackData
	 * argument for each calibration point that was measured by the
	 * calibration process.
	 */
	callback::CallbackPtr	calibrationcallback;

	// the following methods manage the guiding thread
private:
	GuiderProcessPtr	guiderprocess;

public:
	// methods involved with creating a tracker
	double	getPixelsize();
	TrackerPtr	getTracker(const Point& point);
	TrackerPtr	getPhaseTracker();
	TrackerPtr	getDiffPhaseTracker();

public:
	// tracking
	void	startGuiding(TrackerPtr tracker, double interval);
	void	stopGuiding();
	bool	waitGuiding(double timeout);
	double	getInterval();
	const TrackingSummary&	summary();
	
	friend class GuiderProcess;

private:
	/**
	 * \brief Callback for new images
	 *
	 * When the guider operates, it retrieves a new image from the camera
	 * every now and then. To get access to these images, e. g. to allow
	 * a user to monitor the guiding process, this callback can be set
	 * to a callback of type ImageProgramCallback. Every time the guider
	 * gets a new image, it calls this callback with an argument of type
	 * ImageCallbackData.
	 */
	callback::CallbackPtr	_newimagecallback;
public:
	void	newimagecallback(callback::CallbackPtr n) {
		_newimagecallback = n;
	}
	image::ImagePtr	mostRecentImage;
	void	callbackImage(ImagePtr image);

	/**
	 * \brief Callback for tracking information updates
	 *
	 * Whenever the guider process gets a new tracking information,
	 * it uses the lastAction method to inform the guider about
	 * that action. If a callback for last actions is installed, this
	 * information is encapsulated into a callback data structure
	 * and the callback is called with the update information
	 */
private:
	callback::CallbackPtr	_trackingcallback;
	
public:
	void	trackingcallback(callback::CallbackPtr t) {
		_trackingcallback = t;
	}
	void	callbackTrackingPoint(const TrackingPoint& trackingpoint);
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
	// auxiliary functions to simplify the 
#if 0
	camera::CameraPtr	cameraFromName(const std::string& name);
	camera::GuiderPortPtr	guiderportFromName(
						const std::string& name);
#endif
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
