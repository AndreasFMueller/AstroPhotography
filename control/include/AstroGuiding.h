/*
 * AstroGuiding.h -- classes used to implement guiding
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroStar_h
#define _AstroStar_h

#include <AstroImage.h>
#include <AstroAdapter.h>
#include <AstroTransform.h>
#include <AstroCamera.h>

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
	astro::image::transform::Point	operator()(
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
astro::image::transform::Point	StarDetector<Pixel>::operator()(
		const astro::image::ImageRectangle& rectangle,
		unsigned int k) const {
	// work only in the rectangle
	astro::image::WindowAdapter<Pixel>	adapter(image, rectangle);

	// determine the brightest pixel within the rectangle
	astro::image::ImageSize	size = adapter.getSize();
	unsigned	maxx = -1, maxy = -1;
	double	maxvalue = 0;
	for (unsigned int x = 0; x < size.width; x++) {
		for (unsigned int y = 0; y < size.height; y++) {
			double	value = luminance(adapter.pixel(x, y));
			if (value > maxvalue) {
				maxx = x; maxy = y; maxvalue = value;
			}
		}
	}

	// compute the weighted sum of the pixel coordinates in a (2k+1)^2
	// square around the maximum pixel.
	double	xsum = 0, ysum = 0, weightsum = 0;
	for (unsigned int x = maxx - k; x <= maxx + k; x++) {
		for (unsigned int y = maxy - k; y <= maxy + k; y++) {
			double	value = luminance(adapter.pixel(x, y));
			weightsum += value;
			xsum += x * value;
			ysum += y * value;
		}
	}
	xsum /= weightsum;
	ysum /= weightsum;

	// add the offset of the rectangle to get real coordinates
	return astro::image::transform::Point(rectangle.origin.x + xsum,
		rectangle.origin.y + ysum);
}

astro::image::transform::Point	findstar(astro::image::ImagePtr image,
	const astro::image::ImageRectangle& rectangle);

/**
 * \brief Tracker class
 *
 * A tracker keeps track off the offset from an initial state. This is the
 * base class that just defines the interface
 */
class Tracker {
public:
	virtual astro::image::transform::Point	operator()(
			astro::image::ImagePtr newimage) const = 0;
};

/**
 * \brief StarDetector based Tracker
 *
 * This Tracker uses the StarTracker 
 */
class StarTracker : public Tracker {
	astro::image::transform::Point	point;
	astro::image::ImageRectangle rectangle;
	unsigned int	k;
public:
	StarTracker(const astro::image::transform::Point & point,
		const astro::image::ImageRectangle& rectangle,
		unsigned int k);
	virtual astro::image::transform::Point	operator()(
			astro::image::ImagePtr newimage) const = 0;
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
	virtual astro::image::transform::Point	operator()(
			astro::image::ImagePtr newimage) const = 0;
};

/**
 * \brief Guider class
 */
class Guider {
	astro::camera::GuiderPortPtr	guiderport;
	astro::camera::CameraPtr	camera;
public:
	Guider(astro::camera::GuiderPortPtr guiderport, astro::camera::CameraPtr camera);
	bool	calibrate();
};

} // namespace guiding
} // namespace astro

#endif /* _AstroStar_h */
