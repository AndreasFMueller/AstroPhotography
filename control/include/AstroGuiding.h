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
 * \brief Detector class to determine coordinates if a star
 *
 * Star images are not points, they have a distribution. For guiding,
 * we need to determine the coordinates of the star with subpixel accuracy.
 */
template<typename Pixel>
class StarDetector {
	const astro::image::ConstImageAdapter<Pixel>&	image;
public:
	StarDetector(const astro::image::ConstImageAdapter<Pixel>& _image);
	Point	operator()(
		const astro::image::ImageRectangle& rectangle,
		unsigned int k) const;
}; 

/**
 * \brief Create a StarDetector
 */
template<typename Pixel>
StarDetector<Pixel>::StarDetector(
	const astro::image::ConstImageAdapter<Pixel>& _image) : image(_image) {
}

/**
 * \brief Extract Star coordinates
 *
 * By summing the coordinates weighted by luminance around the maximum pixel
 * value in a rectangle, we get the centroid coordinates of the star's
 * response. This is the best estimate for the star coordinates.
 */
template<typename Pixel>
Point	StarDetector<Pixel>::operator()(
		const astro::image::ImageRectangle& rectangle,
		unsigned int k) const {
	// work only in the rectangle
	astro::adapter::WindowAdapter<Pixel>	adapter(image, rectangle);

	// determine the brightest pixel within the rectangle
	astro::image::ImageSize	size = adapter.getSize();
	unsigned	maxx = -1, maxy = -1;
	double	maxvalue = 0;
	for (unsigned int x = 0; x < size.width(); x++) {
		for (unsigned int y = 0; y < size.height(); y++) {
			double	value = luminance(adapter.pixel(x, y));
			if (value > maxvalue) {
				maxx = x; maxy = y; maxvalue = value;
			}
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found maximum at (%d,%d), value = %f",
		maxx, maxy, maxvalue);

	// compute the weighted sum of the pixel coordinates in a (2k+1)^2
	// square around the maximum pixel.
	double	xsum = 0, ysum = 0, weightsum = 0;
	for (unsigned int x = maxx - k; x <= maxx + k; x++) {
		for (unsigned int y = maxy - k; y <= maxy + k; y++) {
			double	value = luminance(adapter.pixel(x, y));
			if (value == value) {
				weightsum += value;
				xsum += x * value;
				ysum += y * value;
			}
		}
	}
	xsum /= weightsum;
	ysum /= weightsum;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "centroid coordinates: %f,%f", xsum, ysum);

	// add the offset of the rectangle to get real coordinates
	return Point(rectangle.origin().x() + xsum,
		rectangle.origin().y() + ysum);
}

Point	findstar(astro::image::ImagePtr image,
	const astro::image::ImageRectangle& rectangle, unsigned int k);

/**
 * \brief Tracker class
 *
 * A tracker keeps track off the offset from an initial state. This is the
 * base class that just defines the interface
 */
class Tracker {
public:
	virtual Point	operator()(astro::image::ImagePtr newimage) const = 0;
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
	astro::image::ImageRectangle _rectangle;
	unsigned int	_k;
public:
	// constructor
	StarTracker(const Point& point,
		const astro::image::ImageRectangle& rectangle,
		unsigned int k);

	// find the displacement
	virtual Point	operator()(
			astro::image::ImagePtr newimage) const;

	// accessors for teh tracker configuration data
	const astro::image::ImageRectangle&	rectangle() const {
		return _rectangle;
	}
	astro::image::ImageRectangle&	rectangle() { return _rectangle; }
	void	rectangle(const astro::image::ImageRectangle& r) {
		_rectangle = r;
	}

	const Point&	point() const { return _point; }
	Point&	point() { return _point; }
	void	point(const Point& p) { _point = p; }

	const unsigned int	k() const { return _k; }
	unsigned int	k() { return _k; }
	void	k(const unsigned int k) { _k = k; }
	
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
	astro::image::ImagePtr	image;
public:
	PhaseTracker(astro::image::ImagePtr image);
	virtual Point	operator()(
			astro::image::ImagePtr newimage) const;
	virtual std::string	toString() const;
};

/**
 * \brief Callback data class for Guider debugging/monitoring
 *
 * Callback data for the guider transports an image
 */
class GuiderNewImageCallbackData : public astro::callback::CallbackData {
	astro::image::ImagePtr	_image;
public:
	GuiderNewImageCallbackData(astro::image::ImagePtr image)
		: _image(image) { }
	astro::image::ImagePtr	image() { return _image; }
};

class GuiderCalibrator;

/**
 * \brief GuiderCalibration
 *
 * The Calibration data. The coefficients in the array a correspond to
 * a matrix that describes how the control commands on the guider port
 * translate into displacements of the guider image.
 */
class GuiderCalibration {
	friend class GuiderCalibrator;
	double	a[6];
	double	det() const;
public:
	GuiderCalibration();
	GuiderCalibration(const double coefficients[6]);
	std::string	toString() const;
	Point	defaultcorrection() const;
	Point	operator()(const Point& offset, double Deltat) const;

	const double&	operator[](size_t index) const;
	double&	operator[](size_t index);

	void	rescale(double scalefactor);
	bool	iscalibrated() const { return 0. != det(); }
};

std::ostream&	operator<<(std::ostream& out, const GuiderCalibration& cal);
std::istream&	operator>>(std::istream& in, GuiderCalibration& cal);

/**
 * \brief GuiderCalibrator
 *
 * The GuiderCalibrator collects a set of points and computes the calibration
 * data from this. The GuiderCalibrator is used by the CalibrationProcess,
 * it adds points during the calibration using the add method, the calibrate
 * method then computes the calibration data.
 */
class GuiderCalibrator {
public:
	class calibration_point {
	public:
		double	t;
		Point	offset;
		Point	point;
		calibration_point(double _t, const Point& _offset,
			const Point& _point) 
			: t(_t), offset(_offset), point(_point) {
		}
	};
private:
	std::vector<calibration_point>	calibration_data;
public:
	GuiderCalibrator();
	void	add(double t, const Point& movement,
			const Point& point);
	GuiderCalibration	calibrate();
};

// we will need the GuiderProcess class, but as we want to keep the 
// implementation (using low level threads and other nasty things) hidden,
// we only define it in the implementation
class GuiderProcess;
typedef std::shared_ptr<GuiderProcess>	GuiderProcessPtr;
class CalibrationProcess;
typedef std::shared_ptr<CalibrationProcess>	CalibrationProcessPtr;

/**
 * \brief enumeration type for the state of the guider
 */
typedef enum { unconfigured, idle, calibrating, calibrated, guiding } GuiderState;

/**
 * \brief State machine class for the Guider
 *
 * The state machine ensures that any change is only accepted only if
 * all prerequisites are met.
 */
class GuiderStateMachine {
	GuiderState	_state;
	const char	*statename() const;
public:
	const GuiderState&	state() const { return _state; }
	operator GuiderState () { return _state; }
	operator const GuiderState () const { return _state; }

	// construct the state machine
	GuiderStateMachine() : _state(astro::guiding::unconfigured) { }

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
	GuiderState	state() const;
	// The guider is essentially composed of a camera and a guiderport
	// we will hardly need access to the camera, but we don't want to
	// loose the reference to it either, so we keep it handy here
private:
	astro::camera::CameraPtr	_camera;
	astro::camera::GuiderPortPtr	_guiderport;
public:
	astro::camera::GuiderPortPtr	guiderport() { return _guiderport; }
	astro::camera::CameraPtr	camera() { return _camera; }
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
	astro::camera::Imager	_imager;
public:
	const astro::camera::Imager&	imager() const { return _imager; }
	astro::camera::Imager&	imager() { return _imager; }
	astro::camera::CcdPtr		ccd() { return _imager.ccd(); }

	/**
	 * \brief Exposure information for guiding images
 	 *
	 * Controlling the exposure parameters includes changing the rectangle
	 * to use during exposure. Since we don't want to implement methods
	 * for all these details, we just expose the exposure structure
	 */
private:
	astro::camera::Exposure	_exposure;
public:
	const astro::camera::Exposure&	exposure() const { return _exposure; }
	astro::camera::Exposure&	exposure() { return _exposure; }
	void	exposure(const astro::camera::Exposure& exposure) {
		_exposure = exposure;
	}

public:
	/**
	 * \brief Construct a guider from camera, ccd, and guiderport
	 *
	 * After construction, the Guider only knows about the hardware it can
	 * use for guiding. There is no information about what parts of 
	 * the image to take into consideration when looking for a guide star,
	 * or even how to expose an image.
	 */
	Guider(astro::camera::CameraPtr camera, astro::camera::CcdPtr ccd,
		astro::camera::GuiderPortPtr guiderport);

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
	 * The focallength and the pixelsize allow to compute reasonable
	 * values for the calibration displacements.
	 * \param tracker	The tracker used for tracking. 
	 * \param focallength	Focallength of the optics used for guiding,
	 *			in m.
	 * \param pixelsize	Pixel size of the CCD chip used for guiding.
	 *			If binning different from 1x1 is used, the
	 *			pixel size must reflect the size of the binned
	 *			pixel. Unit: meters.
	 */
	void	startCalibration(TrackerPtr tracker,
			double focallength = 0, double pixelsize = 0);
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
private:
	CalibrationProcessPtr	calibrationprocess;
	friend class CalibrationProcess;
	void	calibrationCleanup();

	// the following methods manage the guiding thread
private:
	GuiderProcessPtr	guiderprocess;

public:
	// tracking
	void	startGuiding(TrackerPtr tracker, double interval);
	void	stopGuiding();
	bool	waitGuiding(double timeout);
	
	friend class GuiderProcess;

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
	astro::callback::CallbackPtr	newimagecallback;
public:
	astro::image::ImagePtr	mostRecentImage;
	void	callbackImage(ImagePtr image);

	/**
	 * \brief Information about the most recent update
	 */
	void lastAction(double& actiontime, Point& offset, Point& activation);
};
typedef std::shared_ptr<Guider>	GuiderPtr;

/**
 * \brief The GuiderDescriptor is the key to Guiders in the GuiderFactory
 */
class GuiderDescriptor {
	std::string	_cameraname;
	unsigned int	_ccdid;
	std::string	_guiderportname;
public:
	GuiderDescriptor(const std::string& cameraname, unsigned int ccdid,
		const std::string& guiderportname) : _cameraname(cameraname),
		_ccdid(ccdid), _guiderportname(guiderportname) { }
	bool	operator==(const GuiderDescriptor& other) const;
	bool	operator<(const GuiderDescriptor& other) const;
	std::string	cameraname() const { return _cameraname; }
	unsigned int	ccdid() const { return _ccdid; }
	std::string	guiderportname() const { return _guiderportname; }
	std::string	toString() const;
};

/**
 * \brief GuiderFactory class
 */
class GuiderFactory {
	astro::module::Repository	repository;
	typedef	std::map<GuiderDescriptor, GuiderPtr>	guidermap_t;
	guidermap_t	guiders;
	// auxiliary functions to simplify the 
	astro::camera::CameraPtr	cameraFromName(const std::string& name);
	astro::camera::GuiderPortPtr	guiderportFromName(
						const std::string& name);
public:
	GuiderFactory() { }
	GuiderFactory(astro::module::Repository _repository)
		: repository(_repository) { }
	std::vector<GuiderDescriptor>	list() const;
	GuiderPtr	get(const GuiderDescriptor& guiderdescriptor);
};
typedef std::shared_ptr<GuiderFactory>	GuiderFactoryPtr;

} // namespace guiding
} // namespace astro

#endif /* _AstroGuiding_h */
