/*
 * Chart.cpp -- class to create a chart from a set of stars
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroChart.h>
#include <AstroDebug.h>
#include <AstroFormat.h>

using namespace astro::image;

namespace astro {
namespace catalog {

Chart::Chart(const ImageSize& size,
	const RaDec& center, double focallength, double pixelsize)
                : _focallength(focallength),
                  _pixelsize(pixelsize) {
	// create the image
	_image =  new Image<float>(size);
	_imageptr = ImagePtr(_image);
	_image->fill(0);

	// set up parameters for the image
	_maxradius = 7;
	_logarithmic = false;
	_scale = 1;

	// compute the rectangle of the camera
	double	pixelangle = _pixelsize / _focallength;
	Angle	width(size.width() * pixelangle);
	Angle	height(size.height() * pixelangle);
	SkyWindow	window(center, width, height);
	_rectangle = SkyRectangle(window);

	// XXX add the center coordinates to the FITS file
}

Chart::~Chart() {
}

/**
 * \brief draw a set of stars into the chart
 * 
 * \param stars		a set of stars to be drawn inside the image
 */
void	Chart::draw(const Catalog::starset& stars) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create image for %u stars",
		stars.size());

	std::set<Star>::const_iterator	s;
	for (s = stars.begin(); s != stars.end(); s++) {
		try {
			draw(*s);
		} catch (const std::exception& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot map star %s",
				s->toString().c_str());
		} catch (...) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "unknown exception for "
				"star %s", s->toString().c_str());
		}
	}

	// limit the pixel values to 1
	for (int x = 0; x < size().width(); x++) {
		for (int y =0; y < size().height(); y++) {
			if (_image->pixel(x, y) > 1) {
				_image->pixel(x, y) = 1;
			}
		}
	}
}

void	Chart::draw(const Catalog::starsetptr stars) {
	Catalog::starset	*starsp
		= dynamic_cast<Catalog::starset *>(&*stars);
	if (starsp == NULL) {
		throw std::runtime_error("no star set provided");
	}
	draw(*starsp);
}


/**
 * \brief draw a star in the image
 *
 * \param star		the star to be drawn
 */
void	Chart::draw(const Star& star) {

	// compute the pixel coordinates of the star
	astro::Point	p = point(star);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "drawing star %s at %s",
		star.toString().c_str(), p.toString().c_str());

	// compute the radius of the star
	double	I;
	if (_logarithmic) {
		I = 1 - star.mag() / 20;
	} else {
		I = pow(10., -star.mag() / 5);
	}
	I *= _scale;
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "mag = %f, I = %f", star.mag(), I);

	// get the area we have to consider for putting the star image
	int	xmin = p.x() - _maxradius - 1;
	if (xmin < 0) { xmin = 0; }
	int	xmax = p.x() + _maxradius + 1;
	if (xmax > (int)size().width()) { xmax = size().width(); }
	int	ymin = p.y() - _maxradius - 1;
	if (ymin < 0) { ymin = 0; }
	int	ymax = p.y() + _maxradius + 1;
	if (ymax > (int)size().height()) { ymax = size().height(); }
	debug(LOG_DEBUG, DEBUG_LOG, 0, "xrange = [%d, %d), yrange = [%d, %d)",
		xmin, xmax, ymin, ymax);

	// actually put the star image there
	int	counter = 0;
	for (int x = xmin; x < xmax; x++) {
		for (int y = ymin; y < ymax; y++) {
			double	z = hypot(x - p.x(), y - p.y());
			_image->pixel(x, y) += I * pointspreadfunction(z, star.mag());
			counter++;
#if 0
			debug(LOG_DEBUG, DEBUG_LOG, 0, "new value at %d,%d= %f",
				x, y, image.pixel(x,y));
			if (hypot(x - p.x(), y - p.y()) < r) {
				image.pixel(x, y) = 1.;
			}
#endif
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d pixels set", counter);
}

double	Chart::pointspreadfunction(double r, double mag) const {
	if (r > (_maxradius * (20 - mag) / 20.)) {
		return 0;
	}
	return 1000000;
}

static double	sqr(double y) {
	return y * y;
}

/**
 * \brief Get the pixel coordinate point of the star in the window
 *
 * \param window	the sky window where stars should be placed
 * \param star		the star to be placed inside the window
 */
Point	Chart::point(const RaDec& star) const {
	Point	p = _rectangle.map2(star);
	double	x = size().width() * p.x();
	double	y = size().height() * p.y();
	return Point(x, y);
}

/**
 * \brief Get the window around a center point on the sky
 *
 * \param center	get the window of appropriate size
 */
SkyWindow	Chart::getWindow() const {
	return _rectangle.containedin();
}

//////////////////////////////////////////////////////////////////////
// DiffractionChart class implementation
//////////////////////////////////////////////////////////////////////

DiffractionChart::DiffractionChart(const ImageSize& size,
	const RaDec& center, double focallength, double pixelsize)
                : Chart(size, center, focallength, pixelsize) {
	// set the aperture
	aperture(0.280);
}

/**
 * \brief set the aperture
 */
void	DiffractionChart::aperture(double a) {
	_aperture = a;
	_xfactor =  (M_PI * _aperture * _pixelsize)
			/ (_focallength * 0.000000550);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "_xfactor = %f", _xfactor);
}

/**
 * \brief Airy pattern
 */
double	DiffractionChart::pointspreadfunction(double r, double mag) const {
	double	x = _xfactor * r;
	double	a = sqr(2 * j1(x) / x);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%f: airy(%f) = %f", r, x, a);
	return a;
}

//////////////////////////////////////////////////////////////////////
// TurbulenceChart class implementation
//////////////////////////////////////////////////////////////////////
TurbulenceChart::TurbulenceChart(const ImageSize& size, const RaDec& center,
	double focallength, double pixelsize)
		: Chart(size, center, focallength, pixelsize) {
	// set the tolerance value
	turbulence(1);
}

double	TurbulenceChart::pointspreadfunction(double r, double mag) const {
	//return 10 * exp(-sqr(r * sqrt(sqrt((mag < 1) ? 1 : mag)) / _turbulence));
	return exp(-sqr(r / _turbulence));
}

} // namespace catalog
} // namespace astro
