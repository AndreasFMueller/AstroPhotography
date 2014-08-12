/*
 * StereographicChart.cpp -- stereographic chart
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroChart.h>

using namespace astro::image::transform;

namespace astro {
namespace catalog {

StereographicChart::StereographicChart(const RaDec& center,
	unsigned int diameter) : _center(center) {
	_image = new Image<double>(ImageSize(diameter, diameter));
	_imageptr = ImagePtr(_image);
        _image->fill(0);
}

/**
 * \brief Compute a stereographic chart
 */
StereographicChart	StereographicChartFactory::chart(const RaDec& center,
				unsigned int diameter) const {
	// create an image of suitable size
	StereographicChart	chart(center, diameter);

	// get a StereographicProjection for this center
	StereographicProjection	projection(center);

	// get all stars from the catalog (you better don't have the 
	// limiting magnitude too large)
	Catalog::starsetptr	stars = _catalog.find(SkyWindow::all,
					MagnitudeRange(-30, limit_magnitude()));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %u stars", stars->size());

	// draw all the stars
	draw(*(chart._image), projection, stars);

	// give the chart back
	return chart;
}

void	StereographicChartFactory::draw(Image<double>& image,
		const StereographicProjection& projection,
		const Catalog::starsetptr stars) const {
	Catalog::starset        *starsp
		= dynamic_cast<Catalog::starset *>(&*stars);
	if (starsp == NULL) {
		throw std::runtime_error("no star set provided");
	}
	draw(image, projection, *starsp);
}


void	StereographicChartFactory::draw(Image<double>& image,
		const StereographicProjection& projection,
		const Catalog::starset& stars) const {
	Catalog::starset::const_iterator	s;
	for (s = stars.begin(); s != stars.end(); s++) {
		draw(image, projection, *s);
	}
	limit(image, 1.);
}

void	StereographicChartFactory::draw(Image<double>& image,
	const StereographicProjection& projection, const Star& star) const {
	// where
	double	m = image.size().width() / 2;
	Point	p = projection(star) * m + image.size().center();
	ChartFactoryBase::draw(image, p, star);
}

} // namespace catalog
} // namespace astro
