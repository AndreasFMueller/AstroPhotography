/*
 * WindowPredicate.cpp -- Predicate for retrieving stars in a window
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CatalogIterator.h>

namespace astro {
namespace catalog {

WindowPredicate::WindowPredicate(const SkyWindow& window,
                                const MagnitudeRange& magrange)
	: _window(window), _magrange(magrange) {
}

WindowPredicate::~WindowPredicate() {
}

bool	WindowPredicate::operator()(const Star& star) const {
	if (!_magrange.contains(star.mag())) {
		return false;
	}
	return _window.contains(star);
}

} // namespace catalog
} // namespace astro

