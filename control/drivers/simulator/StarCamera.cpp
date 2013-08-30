/*
 * StarCamera.cpp -- star implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Stars.h>
#include <AstroAdapter.h>
#include <Blurr.h>

using namespace astro::image;

namespace astro {

/**
 * \brief compute a random point and add it as a hot pixel position
 */
void	StarCameraBase::addHotPixel() {
	int	x = random() % rectangle().size().width();
	int	y = random() % rectangle().size().height();
	ImagePoint	hotpixel(x, y);
	hotpixels.insert(hotpixel);
}

/**
 * \brief Add a number of hot pixels
 */
void	StarCameraBase::addHotPixels(unsigned int npixels) {
	unsigned int	limit = hotpixels.size() + npixels;
	while (hotpixels.size() < limit) {
		addHotPixel();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera has now %d hot pixels",
		hotpixels.size());
}

/**
 * \brief Compute inverse error function using Newton's algorithm
 *
 * The error function in the C Library is defined as
 *
 * erf(x) = 2/sqrt(pi)*integral from 0 to x of exp(-t*t) dt.
 *
 * The derivative of erf(x) is of course
 *
 * erf'(x) = 2/sqrt(pi) exp(-t*t)
 *
 * 
 */
#define	epsilon	0.000001
#define	maxiterations	10

static double	inverf(double y) {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "y = %f", y);
	double	x = y - 0.5;
	double	m = 2 / sqrt(M_PI);
	double	delta = 1;
	int	counter = 0;
	while ((counter++ < maxiterations) && (delta > epsilon)) {
		delta = (erf(x) - y) / (m * exp(-x * x));
		x -= delta;
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "x = erf(y) = %f", x);
	}
	return x;
}

double	StarCameraBase::noisevalue() const {
	double	x = random() / (double)0xffffffff;
	return _noise * inverf(x);
}



Image<double>	*StarCameraBase::operator()(const StarField& field) const {
	// find out how large we should make the field which we will later
	// transform
	ImageSize	size = rectangle().size();
	ImagePoint	offset;
	if (0 != _radius) {
		size = ImageSize(size.width() + 2 * _radius + 1,
				size.height() + 2 * _radius + 1);
		int	width = 256 * (1 + size.width() / 256);
		int	height = 256 * (1 + size.height() / 256);
		size = ImageSize(width, height);
		offset = ImagePoint(
			(size.width() - rectangle().size().width()) / 2,
			(size.height() - rectangle().size().height()) / 2
		);
	}

	// Here is an ASCII graphic of what we want to accomplish:
	// - The large rectangle is the coordinate rectangle
	// - A is the rectangle we want to image, O is the origin on that
	//   rectangle.
	// - B is the rectangle we need to image if we want to capture
	//   focus blurr without artifacts. The point offset computed above
	//   is the offset of the rectangle A withing B.
	// y
	// +------------------------------------------------+
	// |                                                |
	// |                                                |
	// |               +---------------------+          |
	// |               |B                    |          |
	// |               |     +---------+     |          |
	// |               |     |         |     |          |
	// |               |     |         |     |          |
	// |               |     |    A    |     |          |
	// |               |     |         |     |          |
	// |               |     |         |     |          |
	// |               |     O---------+     |          |
	// |               |                     |          |
	// |               +---------------------+          |
	// |                                                |
	// +------------------------------------------------+ x
	// (0,0)
	// To compute pixels withing the rectangle B. A point (x_B, y_B)
	// has absolute coordinates 
	//
	//    (origin.x() - offset.x() + x_B, origin.y() - offset.y() + y_B)
	//

	// compute a transform based on translation and rotation
	debug(LOG_DEBUG, DEBUG_LOG, 0, "translation = %s, alpha = %f",
		translation().toString().c_str(), alpha());
	astro::image::transform::Transform      transform(alpha(),
							-translation());

	// get the multiplier
	float	multiplier = stretch();

	// fill in the points. 
	ImagePoint	origin = rectangle().origin();
	Image<double>	image(size);
	for (unsigned int x = 0; x < size.width(); x++) {
		for (unsigned int y = 0; y < size.height(); y++) {
			// apply the transform to the current point
			Point   where(origin.x() - offset.x() + x,
					origin.y() - offset.y() + y);
			Point   p = transform(where);

			// compute the intensity
			float  value = 0;
			if (light()) {
				switch (color()) {
				case 0:
					value = field.intensity(p);
					break;
				case 1:
					value = field.intensityR(p);
					break;
				case 2:
					value = field.intensityG(p);
					break;
				case 3:
					value = field.intensityB(p);
					break;
				default:
					break;
				}
				value *= multiplier;
			}

			image.pixel(x, y) = value;
		}
	}

	// compute the blurr if necessary
	if (radius() > 1) {
		Blurr	blurr(radius(), innerradius());
		blurr(image);
	}

	// extract the rectangle 
	ImageRectangle	r(offset, rectangle().size());
	WindowAdapter<double>	wa(image, r);
	Image<double>	*result = new Image<double>(wa);

	return result;
}

} // namespace astro
