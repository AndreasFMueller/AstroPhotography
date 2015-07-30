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

//////////////////////////////////////////////////////////////////////
// Chart implementation
//////////////////////////////////////////////////////////////////////
/**
 * \brief Create a chart
 */
Chart::Chart(const SkyRectangle rectangle, const ImageSize& size)
	: _rectangle(rectangle), _size(size) {
	// create the image
	_image =  new Image<double>(size);
	_imageptr = ImagePtr(_image);
	_image->fill(0);

	// add the metadata
	rectangle.addMetadata(*_image);
}

//////////////////////////////////////////////////////////////////////
// Draw a star on top 
//////////////////////////////////////////////////////////////////////
void	ChartFactoryBase::draw(Image<double>& image, const Point& p,
		const Star& star) const {
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
	if (xmax > (int)image.size().width()) { xmax = image.size().width(); }
	int	ymin = p.y() - _maxradius - 1;
	if (ymin < 0) { ymin = 0; }
	int	ymax = p.y() + _maxradius + 1;
	if (ymax > (int)image.size().height()) { ymax = image.size().height(); }
	debug(LOG_DEBUG, DEBUG_LOG, 0, "xrange = [%d, %d), yrange = [%d, %d)",
		xmin, xmax, ymin, ymax);

	// actually put the star image there
	int	counter = 0;
	for (int x = xmin; x < xmax; x++) {
		for (int y = ymin; y < ymax; y++) {
			double	z = hypot(x - p.x(), y - p.y());
			image.pixel(x, y) += I * pointspreadfunction(z, star.mag());
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

void	ChartFactoryBase::limit(Image<double>& image, double limit) const {
	for (unsigned int x = 0; x < image.size().width(); x++) {
		for (unsigned int y = 0; y < image.size().height(); y++) {
			if (image.pixel(x, y) > limit) {
				image.pixel(x, y) = limit;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// ChartFactory implementation
//////////////////////////////////////////////////////////////////////
/**
 * \brief Create a chart
 *
 * This method creates an image with the geometry required by the the
 * geometry argument, retrieves stars up to the limiting magnitude from
 * the star catalog, adds them to the image and returns everything as a
 * chart.
 */
Chart	ChartFactory::chart(const RaDec& center, const ImageGeometry& geometry) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "chart @%s, geometry=%s",
		center.toString().c_str(), geometry.toString().c_str());
	// first evaluate the geometry and find a rectangle
	SkyRectangle	rectangle(center, geometry);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getting chart for rectangle: %s",
		rectangle.toString().c_str());

	// create the Chart
	Chart	chart(rectangle, geometry);

	// next find a window to get all the stars in the window
	SkyWindow	window = rectangle.containedin();
	Catalog::starsetptr	stars = _catalog->find(window,
                                        MagnitudeRange(-30, limit_magnitude()));

	// add the stars to the image
	draw(*chart._image, rectangle, stars);

	// return the completed chart
	return chart;
}


/**
 * \brief draw a set of stars into the chart
 * 
 * \param stars		a set of stars to be drawn inside the image
 */
void	ChartFactory::draw(Image<double>& image, const SkyRectangle& rectangle,
		const Catalog::starset& stars) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create image for %u stars",
		stars.size());

	std::set<Star>::const_iterator	s;
	for (s = stars.begin(); s != stars.end(); s++) {
		try {
			draw(image, rectangle, *s);
		} catch (const std::exception& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot map star %s",
				s->toString().c_str());
		} catch (...) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "unknown exception for "
				"star %s", s->toString().c_str());
		}
	}

	// limit the pixel values to 1
	limit(image, 1.);
}

/**
 * \brief Draw a sets of of stars to the chart
 */
void	ChartFactory::draw(Image<double>& image, const SkyRectangle& rectangle,
		const Catalog::starsetptr stars) const {
	Catalog::starset	*starsp
		= dynamic_cast<Catalog::starset *>(&*stars);
	if (starsp == NULL) {
		throw std::runtime_error("no star set provided");
	}
	draw(image, rectangle, *starsp);
}


/**
 * \brief draw a star in the image
 *
 * \param star		the star to be drawn
 */
void	ChartFactory::draw(Image<double>& image, const SkyRectangle& rectangle,
		const Star& star) const {

	// compute the pixel coordinates of the star
	astro::Point	p = rectangle.point(image.size(), star);

	// draw the star at this point
	ChartFactoryBase::draw(image, p, star);
#if 0
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
	if (xmax > (int)image.size().width()) { xmax = image.size().width(); }
	int	ymin = p.y() - _maxradius - 1;
	if (ymin < 0) { ymin = 0; }
	int	ymax = p.y() + _maxradius + 1;
	if (ymax > (int)image.size().height()) { ymax = image.size().height(); }
	debug(LOG_DEBUG, DEBUG_LOG, 0, "xrange = [%d, %d), yrange = [%d, %d)",
		xmin, xmax, ymin, ymax);

	// actually put the star image there
	int	counter = 0;
	for (int x = xmin; x < xmax; x++) {
		for (int y = ymin; y < ymax; y++) {
			double	z = hypot(x - p.x(), y - p.y());
			image.pixel(x, y) += I * pointspreadfunction(z, star.mag());
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
#endif
}

double	CirclePointSpreadFunction::operator()(double r, double mag) const {
	if (r > (_maxradius * (20 - mag) / 20.)) {
		return 0;
	}
	return 1000000;
}

static double	sqr(double y) {
	return y * y;
}

//////////////////////////////////////////////////////////////////////
// DiffractionPointSpreadFunction class implementation
//////////////////////////////////////////////////////////////////////

DiffractionPointSpreadFunction::DiffractionPointSpreadFunction(
	const ImageGeometry& geometry, double aperture) {
	_aperture = aperture;
	_xfactor =  (M_PI * _aperture * geometry.pixelsize())
			/ (geometry.focallength() * 0.000000550);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "_xfactor = %f", _xfactor);
}

double	DiffractionPointSpreadFunction::operator()(double r, double /* mag */) const {
	double	x = _xfactor * r;
	double	a = sqr(2 * j1(x) / x);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%f: airy(%f) = %f", r, x, a);
	return a;
}

//////////////////////////////////////////////////////////////////////
// TurbulencePointSpreadFunction class implementation
//////////////////////////////////////////////////////////////////////
double	TurbulencePointSpreadFunction::operator()(double r, double /* mag */) const {
	return exp(-sqr(r / _turbulence));
}

} // namespace catalog
} // namespace astro
