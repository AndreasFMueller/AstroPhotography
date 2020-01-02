/*
 * PsfExtractor.cpp
 *
 * (c) 2019 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroPsf.h>
#include <AstroAdapter.h>
#include <AstroTransform.h>
#include <AstroFilter.h>

namespace astro {
namespace psf {

//////////////////////////////////////////////////////////////////////
// A criterion class that accepts only unclipped stars
//////////////////////////////////////////////////////////////////////

class BrightnessCriterion : public transform::StarAcceptanceCriterion {
	double	_brightness;
public:
	BrightnessCriterion(const ConstImageAdapter<double>& image,
		double brightness)
		: StarAcceptanceCriterion(image), _brightness(brightness) {
	}
	BrightnessCriterion(const ConstImageAdapter<double>& image)
		: StarAcceptanceCriterion(image) {
		// get the maximum
		_brightness = 0.8 * filter::Max<double, double>().filter(image);
	}
	bool	accept(const transform::Star& star) const {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "checking %s brightness %.1f",
			star.toString().c_str(), _brightness);
		return star.brightness() < _brightness;
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

	// build a suitable criterion for stars to be acceptable
	BrightnessCriterion	criterion(luminance);

	// 1. extract stars
	transform::StarExtractor	extractor(_maxstars, _radius);
	std::vector<transform::Star>	stars = extractor.stars(luminance,
						criterion);
	if (debuglevel > 0) {
		for (auto i = stars.begin(); i != stars.end(); i++) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "extracted star: %s",
				i->toString().c_str());
		}
	}

	// 2. build 
	return NULL;
}

} // namespace psf
} // namespace astro
