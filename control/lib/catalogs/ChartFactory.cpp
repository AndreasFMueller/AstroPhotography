/*
 * ChartFactory.cpp -- Chart Factory implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroChart.h>
#include <AstroDebug.h>
#include <AstroFormat.h>

using namespace astro::image;

namespace astro {
namespace catalog {

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

	// apply the point spread function
	int	morepixels = 100;
	spread(*chart._image, morepixels, geometry);

	// limit the pixel values to 1
	limit(*chart._image, 1.);

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
}

} // namespace catalog
} // namespace astro
