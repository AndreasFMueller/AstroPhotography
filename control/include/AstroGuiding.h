/*
 * AstroGuiding.h -- classes used to implement guiding
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroStar_h
#define _AstroStar_h

#include <AstroImage.h>
#include <AstroTypes.h>
#include <AstroAdapter.h>
#include <AstroTransform.h>
#include <AstroCamera.h>
#include <AstroDebug.h>
#include <AstroImager.h>
#include <AstroCallback.h>

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
 */
class Guider {
	astro::camera::GuiderPortPtr	guiderport;
	astro::camera::Imager	imager;
	astro::camera::Exposure	exposure;
	GuiderCalibration	calibration;
	double	gridconstant;
	bool	calibrated;
	void	sleep(double t);
	void	moveto(double ra, double dec);
	GuiderProcessPtr	guiderprocess;
public:
	Guider(astro::camera::GuiderPortPtr guiderport,
		astro::camera::Imager imager);
	// XXX we also need a constructor that recovers previously recorded
	//     calibration data

	// controlling the exposure parameters
	const astro::camera::Exposure&	getExposure() const;
	void	setExposure(const astro::camera::Exposure& exposure);

	// getting images
	void	startExposure();
	ImagePtr	getImage();

	// calibration
	bool	calibrate(TrackerPtr tracker,
		double focallength = 0, double pixelsize = 0);
	const GuiderCalibration&	getCalibration() const;

	astro::camera::Imager	getImager();
	astro::camera::GuiderPortPtr	getGuiderPort();

	// tracking
	bool	start(TrackerPtr tracker);
	bool	stop();
	friend class GuiderProcess;

	// callbacks
	astro::callback::CallbackPtr	newimagecallback;
	
};

} // namespace guiding
} // namespace astro

#endif /* _AstroStar_h */
