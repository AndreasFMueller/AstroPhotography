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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "centroid offsets: %f,%f", xsum, ysum);

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
};

typedef std::tr1::shared_ptr<Tracker>	TrackerPtr;

/**
 * \brief StarDetector based Tracker
 *
 * This Tracker uses the StarTracker 
 */
class StarTracker : public Tracker {
	Point	point;
	astro::image::ImageRectangle rectangle;
	unsigned int	k;
public:
	StarTracker(const Point& point,
		const astro::image::ImageRectangle& rectangle,
		unsigned int k);
	virtual Point	operator()(
			astro::image::ImagePtr newimage) const;
	const astro::image::ImageRectangle&	getRectangle() const;
};

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
};

/**
 * \brief Callback class for Guider debugging/monitoring
 */
class GuiderNewImageCallbackData : public astro::callback::CallbackData {
	astro::image::ImagePtr	_image;
public:
	GuiderNewImageCallbackData(astro::image::ImagePtr image)
		: _image(image) { }
	astro::image::ImagePtr	image() { return _image; }
};

/**
 * \brief GuiderCalibration
 */
class GuiderCalibration {
public:
	double	a[6];
	std::string	toString() const;
	Point	defaultcorrection() const;
	Point	operator()(const Point& offset, double Deltat) const;
};

/**
 * \brief GuiderCalibrator
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
typedef std::tr1::shared_ptr<GuiderProcess>	GuiderProcessPtr;

/**
 * \brief Guider class
 * 
 * The guider class unifies all the operations needed for guiding.
 */
class Guider {
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
	// Imaging is performed via the imager, but the imager is built from
	// from the ccd and the some additional data like darks and flats.
	// The imager is exposed so that the client can change the imager
	// parameters, e. g. can add a dark image, or enable pixel
	// interpolation
	astro::camera::Imager	_imager;
public:
	const astro::camera::Imager&	imager() const { return _imager; }
	astro::camera::Imager&	imager() { return _imager; }
	astro::camera::CcdPtr		ccd() { return _imager.ccd(); }

	// Controlling the exposure parameters includes changing the rectangle
	// to use during exposure. Since we don't want to implement methods
	// for all these details, we just expose the exposure structure
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
	 */
	Guider(astro::camera::CameraPtr camera, astro::camera::CcdPtr ccd,
		astro::camera::GuiderPortPtr guiderport);

	// We should be able to get images through the imager, using the
	// previously defined exposure structure.
public:
	void	startExposure();
	ImagePtr	getImage();

	// The following members are used for the calibration of the guider.
	// The guider calibration is encapsulated in a GuiderCalibration
	// object, the calibration step can be bypassed by setting the
	// the calibration directly.
private:
	GuiderCalibration	_calibration;
public:
	const GuiderCalibration&	calibration() const { return _calibration; }
	GuiderCalibration&	calibration() { return _calibration; }
	void	calibration(const GuiderCalibration& calibration);
	/**
	 * \brief Perform guider calibration
	 * 
	 * This method takes a tracker object that is programmed to follow
	 * a given star, and performs the calibration using that star.
	 * It needs information about the focal length of the telescope
	 * to determine the expected effect of the guider port commands
	 * on the star position detected by the tracker. Without this
	 * information the tracker might loose the star it is expected
	 * to track.
	 */
	bool	calibrate(TrackerPtr tracker,
		double focallength = 0, double pixelsize = 0);
private:
	// here come a few private variables and methods to help with the
	// calibration process
	double	gridconstant;
	bool	calibrated;
	void	sleep(double t);
	void	moveto(double ra, double dec);

	// the following methods 
private:
	GuiderProcessPtr	guiderprocess;

public:
	// tracking
	bool	start(TrackerPtr tracker);
	bool	stop();
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
};
typedef std::tr1::shared_ptr<Guider>	GuiderPtr;

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
	std::vector<GuiderDescriptor>	list() const;
	GuiderPtr	get(const GuiderDescriptor& guiderdescriptor);
};
typedef std::tr1::shared_ptr<GuiderFactory>	GuiderFactoryPtr;

} // namespace guiding
} // namespace astro

#endif /* _AstroGuiding_h */
