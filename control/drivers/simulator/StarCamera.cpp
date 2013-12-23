/*
 * StarCamera.cpp -- star implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Stars.h>
#include <AstroAdapter.h>
#include <Blurr.h>

using namespace astro::image;
using namespace astro::camera;
using namespace astro::adapter;

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


void    StarCameraBase::noise(const double& n) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set noise value to %f", n);
	_noise = n;
}

/**
 * \brief Compute the image of a star field
 *
 * This method computes the distribution of the stars, with appropriate
 * transformations, and the effect of the focuser.
 */
Image<double>	*StarCameraBase::operator()(const StarField& field) const {
	// find out how large we should make the field which we will later
	// transform. This must be large enough so that we catch starts that 
	// are just ouside the image area, because the will show up when
	// the image is out of focus.
	ImageSize	size = rectangle().size();
	ImagePoint	offset;
	if (0 != _radius) {
		size = ImageSize(size.width() + 2 * _radius + 1,
				size.height() + 2 * _radius + 1);
		// we need to ensure that the size is a multiple of
		// 256 so that the Blurr will work
		int	width = 256 * (1 + size.width() / 256);
		int	height = 256 * (1 + size.height() / 256);
		size = ImageSize(width, height);
		offset = ImagePoint(
			(size.width() - rectangle().size().width()) / 2,
			(size.height() - rectangle().size().height()) / 2
		);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image size: %s, offset: %s",
			size.toString().c_str(), offset.toString().c_str());
	}

	// Here is an ASCII graphic of what we want to accomplish:
	// - The large rectangle is the coordinate rectangle
	// - A is the rectangle we want to image, O is the origin on that
	//   rectangle.
	// - B is the rectangle we need to image if we want to capture
	//   focus blurr without artifacts. The point offset computed above
	//   is the offset of the rectangle A withing B.
	// y-axis
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
	// +------------------------------------------------+ x-axis
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

	// add noise to the image rectangle
	if (noise()) {
		addnoise(*result);
	}

	return result;
}

/**
 * \brief Add noise to the image
 */
void	StarCameraBase::addnoise(Image<double>& image) const {
	unsigned int	width = image.size().width();
	unsigned int	height = image.size().height();
	for (unsigned int x = 0; x < width; x++) {
		for (unsigned int y = 0; y < height; y++) {
			image.pixel(x, y) += noisevalue();
		}
	}
}

/**
 * \brief Rescale the image
 *
 * Rescale the image so that all pixel values lie between 0 and the
 * scale argument
 */
void	StarCameraBase::rescale(Image<double>& image, double scale) const {
	unsigned int	width = image.size().width();
	unsigned int	height = image.size().height();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rescaling %dx%d image", width, height);
	for (unsigned int x = 0; x < width; x++) {
		for (unsigned int y = 0; y < height; y++) {
			double	value = scale * image.pixel(x, y);
			if (value > scale) {
				value = scale;
			}
			image.pixel(x, y) = value;
		}
	}
}

/**
 * \brief Add hot pixels to the image
 */
void	StarCameraBase::addhot(Image<double>& image, double hotvalue) const {
	ImagePoint	origin = rectangle().origin();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add hot pixels to %s image",
		image.getFrame().toString().c_str());
	std::set<ImagePoint>::const_iterator	i;
	for (i = hotpixels.begin(); i != hotpixels.end(); i++) {
		if (rectangle().contains(*i)) {
			fill0(image, (*i) - origin, hotvalue);
/*
			image.pixel(i->x() - origin.x(), i->y() - origin.y())
				= hotvalue;
*/
		}
	}
}

/**
 * \brief Computed binned pixel values
 */
double	StarCameraBase::bin0(Image<double>& image,
		unsigned int x, unsigned int y) const {
	// find out whether we are at the edge of the image, where we may not
	// be able to bin a full image
	unsigned int	maxx = image.getFrame().size().width() - x;
	if (maxx > binning().getX()) {
		maxx = binning().getX();
	}
	unsigned int	maxy = image.getFrame().size().height() - y;
	if (maxy > binning().getY()) {
		maxy = binning().getY();
	}
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "maxx = %d, maxy = %d", maxx, maxy);
	double	value = 0;
	for (unsigned int xoffset = 0; xoffset < maxx; xoffset++) {
		for (unsigned int yoffset = 0; yoffset < maxy; yoffset++) {
			value += image.pixel(x + xoffset, y + yoffset);
		}
	}
	return value;
}

/**
 * \brief Perform binning
 */
void	StarCameraBase::bin(Image<double>& image) const {
	unsigned int	width = image.size().width();
	unsigned int	height = image.size().height();
	unsigned int	deltax = binning().getX();
	unsigned int	deltay = binning().getY();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%dx%d-binning of %dx%d image",
		deltax, deltay, width, height);
	for (unsigned int x = 0; x < width; x += deltax) {
		for (unsigned int y = 0; y < height; y += deltay) {
			image.pixel(x, y) = bin0(image, x, y);
		}
	}
}

/**
 * \brief Fill a binned pixel with a given value
 */
void	StarCameraBase::fill0(Image<double>& image, const ImagePoint& point,
		double fillvalue) const {
	ImagePoint	corner = (point / binning()) * binning();
	for (unsigned int x = 0; x < binning().getX(); x++) {
		for (unsigned int y = 0; y < binning().getY(); y++) {
			image.pixel(corner.x() + x, corner.y() + y)
				= fillvalue;
		}
	}
}

} // namespace astro
