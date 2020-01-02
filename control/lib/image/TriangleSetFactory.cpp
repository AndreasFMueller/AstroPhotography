/*
 * TriangleSetFactory
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTransform.h>
#include <AstroDebug.h>

namespace astro {
namespace image {
namespace transform {

TriangleSetFactory::TriangleSetFactory() {
	_radius = 16;
	_numberofstars = 20;
}

bool	TriangleSetFactory::good(const Triangle& t, double l) const {
	if (t.longside() < l) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "long side %f too short",
			t.longside());
		return false;
	}
	if ((t.middleside() < 0.6)
		|| (t.middleside() > 0.9)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "middle side bad: %f",
			t.middleside());
		return false;
	}
	if (t.angle() > 0.3 * M_PI) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "angle too large %f degrees",
			t.angle() * 180 / M_PI);
		return false;
	}
	if (t.angle() < -0.3 * M_PI) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "angle too small %f degrees",
			t.angle() * 180 / M_PI);
		return false;
	}
	return true;
}

TriangleSet	TriangleSetFactory::get(ImagePtr image) const {
	// find the lower limit for triangle side
	double	limit = (image->size().width() + image->size().height()) / 20;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "lengthlimit: %f", limit);

	// construct a LuminanceExtractor to build the criterion on
	adapter::LuminanceExtractor	luminance(image);
	StarAcceptanceCriterion	criterion(luminance);

	// first lets get a set of stars
	StarExtractor	extractor(_numberofstars, _radius);
	std::vector<Star>	stars = extractor.stars(image, criterion);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing triangles from %d stars",
		stars.size());

	return get(stars, limit);
}

TriangleSet	TriangleSetFactory::get(const ConstImageAdapter<double>& image) const {
	// find the lower limit for triangle side
	double	limit = (image.getSize().width() + image.getSize().height()) / 20;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "lengthlimit: %f", limit);

	// construct a LuminanceExtractor to build the criterion on
	StarAcceptanceCriterion	criterion(image);

	StarExtractor	extractor(_numberofstars, _radius);
	std::vector<Star>	stars = extractor.stars(image, criterion);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing triangles from %d stars",
		stars.size());

	return get(stars, limit);
}

/**
 * \brief Convert a star set into a triangle set
 */
TriangleSet	TriangleSetFactory::get(const std::vector<Star>& stars,
			double l) const {
	TriangleSet	result;
	int	i = 0;
	for (auto ptr = stars.begin(); ptr != stars.end(); ptr++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "Star[%d] %s", i++,
			ptr->toString().c_str());
	}

	// now go through the whole image and produce triangles
	std::vector<Star>::const_iterator	p1, p2, p3;
	for (p1 = stars.begin(); p1 != stars.end(); p1++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "p1 = %s",
			p1->toString().c_str());
		auto ptr = p1 + 1;
		if (ptr == stars.end()) {
			continue;
		}
		for (p2 = ptr; p2 != stars.end(); p2++) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "p2 = %s",
				p2->toString().c_str());
			auto	ptr = p2 + 1;
			if (ptr == stars.end()) {
				continue;
			}
			for (p3 = ptr; p3 != stars.end(); p3++) {
				debug(LOG_DEBUG, DEBUG_LOG, 0, "p3 = %s",
					p3->toString().c_str());
				Triangle	t(*p1, *p2, *p3);
				if (good(t, l)) {
					result.insert(result.begin(), t);
				}
			}
		}
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d triangles", result.size());
	return result;
}

} // namespace transform
} // namespace image
} // namespace astro
