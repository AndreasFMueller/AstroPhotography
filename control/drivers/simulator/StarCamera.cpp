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
 * \brief Constructor for the StarCameraBase
 *
 * sets the _content variable depending on the environment variable STARCONTENT
 */
StarCameraBase::StarCameraBase(const ImageRectangle& rectangle)
	: _content(STARS), _rectangle(rectangle), _stretch(1),
	  _dark(0), _noise(0), _light(true), _color(0), _radius(0),
	  _innerradius(0) {
	// check environement variable
	char	*v = getenv("STARCONTENT");
	if (NULL == v) {
		return;
	}
	std::string	contentvar(v);
	if (contentvar == "SUN") {
		_content = SUN;
	}
	if (contentvar == "PLANET") {
		_content = PLANET;
	}
}

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


void    StarCameraBase::noise(double n) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set noise value to %f", n);
	_noise = n;
}

/**
 * \brief Compute the image of a star field
 *
 * This method computes the distribution of the stars, with appropriate
 * transformations, and the effect of the focuser.
 */
Image<double>	*StarCameraBase::doubleImage(StarField& field) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start building base image");
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "translation = %s",
		translation().toString().c_str());

	// fill in the points. 
	ImagePoint	origin = rectangle().origin();
	Point	shift = Point(origin - offset) - translation();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "shift = %s",
		shift.toString().c_str());

	Image<double>	image(size);
	for (int x = 0; x < size.width(); x++) {
		for (int y = 0; y < size.height(); y++) {
			image.pixel(x, y) = 0;
		}
	}

	// if this is a light image, we have to expose the stars from the
	// star field
	if (light()) {
		// depending on the content, call the methods to expose
		switch (_content) {
		case STARS:
			addStarIntensities(image, field, shift);
			break;
		case SUN:
			addSunIntensity(image, shift);
			break;
		case PLANET:
			addPlanetIntensity(image, shift);
			break;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "object values applied");

		// compute the blurr if necessary
		if (radius() > 1) {
			Blurr	blurr(radius(), innerradius());
			Image<double>	blurredimage = blurr(image);
			image = blurredimage;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "blurring completed");
	}

	// extract the rectangle 
	ImageRectangle	r(offset, rectangle().size());
	WindowAdapter<double>	wa(image, r);
	Image<double>	*result = new Image<double>(wa);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rectangle %s extracted",
		r.toString().c_str());

	// stretch the values
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stretch factor = %.1f", _stretch);
	int	width = r.size().width();
	int	height = r.size().height();
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			result->pixel(x, y) = _stretch * result->pixel(x,y);
		}
	}

	// add noise to the image rectangle
	if (noise()) {
		addnoise(*result);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "noise added");
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "base image complete");
	return result;
}

/**
 * \brief Add the intensity of a body of a given radius
 */
void	StarCameraBase::addBodyIntensity(Image<double>& image,
		const Point& shift, int radius) const {
	int	w = image.size().width();
	int	h = image.size().height();
	ImagePoint	body = image.size().center();
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			// apply the shift to the current point
			Point   p(shift.x() + x, shift.y() + y);
			double  value = 0;
			double	r = (p - body).abs();
			if (r < radius) {
				value = 1.;
			} else if (r > (radius + 2)) {
				value = 0;
			} else {
				// interpolate between 100 and 102
				value = (radius + 2 - r) / 2;
			}
			image.pixel(x, y) = value;
		}
	}
}

/**
 * \brief Add intensity for a simulated planet
 */
void	StarCameraBase::addPlanetIntensity(Image<double>& image,
		const Point& shift) const {
	addBodyIntensity(image, shift, 10);
}

/**
 * \brief Add intensity for a simulated sun
 */
void	StarCameraBase::addSunIntensity(Image<double>& image,
		const Point& shift) const {
	addBodyIntensity(image, shift, 100);
}

/**
 * \brief Add the intensity of one particular star
 */
void	StarCameraBase::addStarIntensity(Image<double>& image,
		StellarObjectPtr star, Point shift) const {
	// compute the position on the image
	ImagePoint	c(star->position().x() - shift.x(),
				star->position().y() - shift.y());

	// depending on the orientation, we have to flip the star
	Star	*starp = dynamic_cast<Star *>(&*star);
	if (NULL == starp) {
		return;
	}
	Star	fstar = *starp;
	if (!_west) {
		// flip the image point
		c = image.size().flip(c);

		// flip the star
		Point	p = fstar.position();
		Point	flipped(image.size().width() - 1 - p.x(),
				image.size().height() - 1 - p.y());
		fstar.position(flipped);

		// flip the shift
#if 1
		shift = -shift;
#endif
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add star at c = %s, p = %s, shift = %s",
		c.toString().c_str(), fstar.position().toString().c_str(),
		shift.toString().c_str());

	// compute the area around the star for which we have to modify
	// the image to add the diffraction image of the star
	int	xmin = c.x() - 30;
	if (xmin < 0) { xmin = 0; }

	int	xmax = c.x() + 31;
	if (xmax > image.size().width()) { xmax = image.size().width(); }

	int	ymin = c.y() - 30;
	if (ymin < 0) { ymin = 0; }

	int	ymax = c.y() + 31;
	if (ymax > image.size().height()) { ymax = image.size().height(); }

	// update the environment
	for (int x = xmin; x < xmax; x++) {
		for (int y = ymin; y < ymax; y++) {
			double	value = image.pixel(x,y);
			Point	p(shift.x() + x, shift.y() + y);
			switch (color()) {
			case 0:
				value += fstar.intensity(p);
				break;
			case 1:
				value += fstar.intensityR(p);
				break;
			case 2:
				value += fstar.intensityG(p);
				break;
			case 3:
				value += fstar.intensityB(p);
				break;
			default:
				break;
			}
			image.pixel(x, y) = value;
		}
	}
}

/**
 * \brief Add intensities of all the stars
 */
void	StarCameraBase::addStarIntensities(Image<double>& image,
		const StarField& field,
		const Point& shift) const {
	ImageSize	size = image.size();
	for (unsigned int i = 0; i < field.nObjects(); i++) {
		// now add the object intensity for object i
		addStarIntensity(image, field[i], shift);
	}
}

/**
 * \brief Add noise to the image
 */
void	StarCameraBase::addnoise(Image<double>& image) const {
	int	width = image.size().width();
	int	height = image.size().height();
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			double	n = noisevalue();
			image.pixel(x, y) = image.pixel(x, y) + n;
		}
	}
}

/**
 * \brief Rescale the image
 *
 * Rescale the image so that all pixel values lie between 0 and the
 * scale argument. This is done by cutting of all values below 0
 * and above <scale>
 */
void	StarCameraBase::rescale(Image<double>& image, double scale) const {
	int	width = image.size().width();
	int	height = image.size().height();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rescaling %dx%d image with scale %f",
		width, height, scale);
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			double	value = scale * image.pixel(x, y);
			if (value > scale) {
				value = scale;
			}
			if (value < 0) {
				value = 0;
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
double	StarCameraBase::bin0(Image<double>& image, int x, int y) const {
	// find out whether we are at the edge of the image, where we may not
	// be able to bin a full image
	int	maxx = image.getFrame().size().width() - x;
	if (maxx > binning().x()) {
		maxx = binning().x();
	}
	int	maxy = image.getFrame().size().height() - y;
	if (maxy > binning().y()) {
		maxy = binning().y();
	}
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "maxx = %d, maxy = %d", maxx, maxy);
	double	value = 0;
	for (int xoffset = 0; xoffset < maxx; xoffset++) {
		for (int yoffset = 0; yoffset < maxy; yoffset++) {
			value += image.pixel(x + xoffset, y + yoffset);
		}
	}
	return value;
}

/**
 * \brief Perform binning
 */
void	StarCameraBase::bin(Image<double>& image) const {
	int	width = image.size().width();
	int	height = image.size().height();
	unsigned int	deltax = binning().x();
	unsigned int	deltay = binning().y();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%dx%d-binning of %dx%d image",
		deltax, deltay, width, height);
	for (int x = 0; x < width; x += deltax) {
		for (int y = 0; y < height; y += deltay) {
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
	for (int x = 0; x < binning().x(); x++) {
		for (int y = 0; y < binning().y(); y++) {
			image.pixel(corner.x() + x, corner.y() + y)
				= fillvalue;
		}
	}
}

} // namespace astro
