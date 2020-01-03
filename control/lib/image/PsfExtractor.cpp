/*
 * PsfExtractor.cpp
 *
 * (c) 2019 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroPsf.h>
#include <AstroAdapter.h>
#include <AstroTransform.h>
#include <AstroFilter.h>
#include <AstroIO.h>

namespace astro {
namespace psf {

//////////////////////////////////////////////////////////////////////
// A criterion class that accepts only unclipped stars
//////////////////////////////////////////////////////////////////////

class BrightnessCriterion : public transform::StarAcceptanceCriterion {
	double	_minimum;
	double	_brightness;
public:
	double	brightness() const { return _brightness; }
	BrightnessCriterion(const ConstImageAdapter<double>& image,
		double minimum, double brightness)
		: StarAcceptanceCriterion(image),
		  _minimum(minimum), _brightness(brightness) {
	}
	BrightnessCriterion(const ConstImageAdapter<double>& image)
		: StarAcceptanceCriterion(image) {
		// get the maximum
		_brightness = 0.8 * filter::Max<double, double>().filter(image);
		_minimum = 0.1 * _brightness;
	}
	bool	accept(const transform::Star& star) const {
		ImagePoint	s(star.x(), star.y());
		double	v = _image.pixel(s);
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"checking %s, value=%f, brightness %.1f",
			star.toString().c_str(), v, _brightness);
		if (_minimum >= star.brightness()) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"star not bright enough");
			return false;
		}
		if (star.brightness() >= _brightness) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "star is too bright");
			return false;
		}
		return true;
	}
};


/**
 * \brief Build a point spread function extractor
 */
PsfExtractor::PsfExtractor() : _radius(30), _maxstars(10) {
}

/**
 * \brief Extract the point spread function from an image
 *
 * \param image		The image to extract the psf from
 */
image::Image<double>	*PsfExtractor::extract(image::ImagePtr image) {
	// construct luminance
	adapter::LuminanceExtractor	luminance(image);
	ImageSize	s = luminance.getSize();
	Point	c = Point(s.width(), s.height()) * 0.5;

	// construct an image for reporting
	Image<RGB<double> >	report(s);
	for (int x = 0; x < s.width(); x++) {
		for (int y = 0; y < s.height(); y++) {
			report.pixel(x, y) = luminance.pixel(x, y);
		}
	}

	// build a suitable criterion for stars to be acceptable
	BrightnessCriterion	criterion(luminance);

	// 1. extract stars
	transform::StarExtractor	extractor(_maxstars, _radius);
	std::vector<transform::Star>	stars = extractor.stars(luminance,
						criterion);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "isolated stars found: %d",
		stars.size());
	if (debuglevel > 0) {
		for (auto i = stars.begin(); i != stars.end(); i++) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "extracted star: %s",
				i->toString().c_str());
		}
	}

	// 2. draw the stars into the report image
	double	maxvalue = 0;
	for (auto i = stars.begin(); i != stars.end(); i++) {
		int	x0 = i->x();
		int	y0 = i->y();
		RGB<double>	red(criterion.brightness() / 0.8, 0., 0.);
		for (int x = x0 - 10; x <= x0 + 10; x++) {
			report.pixel(x, y0) = red;
		}
		for (int y = y0 - 10; y <= y0 + 10; y++) {
			report.pixel(x0, y) = red;
		}
		if (i->brightness() > maxvalue) {
			maxvalue = i->brightness();
		}
	}
	io::FITSoutfile<RGB<double> >	reportout("report.fits");
	reportout.setPrecious(false);
	reportout.write(report);

	// 3. build the Psf image
	Image<double>	*psf = new Image<double>(image->size());
	psf->fill(0.);

	// 4. add all images 
	int	xmin = c.x() - _radius;
	int	xmax = c.x() + _radius;
	int	ymin = c.y() - _radius;
	int	ymax = c.y() + _radius;
	for (auto i = stars.begin(); i != stars.end(); i++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "add star %s",
			i->toString().c_str());
		Point	p = c - *i;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "translating by %s",
			p.toString().c_str());
		image::transform::TranslationAdapter<double>	ta(luminance, p);
		for (int x = xmin; x < xmax; x++) {
			for (int y = ymin; y < ymax; y++) {
				double	v = psf->pixel(x, y);
				v = v + ta.pixel(x, y);
				psf->pixel(x, y) = v;
			}
		}
	}

	// 5. find the value that is smaller than more than 99% of the pixels
	// XXX currently we just take the minium
	double	minvalue = maxvalue;
	for (int x = xmin; x < xmax; x++) {
		for (int y = ymin; y < ymax; y++) {
			double	v = psf->pixel(x, y);
			if (v < minvalue) {
				minvalue = v;
			}
		}
	}

	// 6. subtract the minimum and flatten the residual
	debug(LOG_DEBUG, DEBUG_LOG, 0, "subtracting floor %f", minvalue);
	double	sigma = _radius / 6.;
	sigma = sigma * sigma;
	double	pedestal = 0.01 * (maxvalue - minvalue) + minvalue;
	double	sum = 0;
	for (int x = xmin; x < xmax; x++) {
		for (int y = ymin; y < ymax; y++) {
			double	d = hypot(x - c.x(), y - c.y()) - _radius / 2;
			if (d < 0) { d = 0; }
			double	v = psf->pixel(x, y) - pedestal;
			if (v < 0) {
				v = 0;
			}
			v *= exp(-d * d / sigma);
			psf->pixel(x, y) = v;
			sum += v;
		}
	}

	// 7. Normalize the image
	for (int x = xmin; x < xmax; x++) {
		for (int y = ymin; y < ymax; y++) {
			double	v = psf->pixel(x, y) / sum;
			psf->pixel(x, y) = v;
		}
	}

	// done
	return psf;
}

} // namespace psf
} // namespace astro
