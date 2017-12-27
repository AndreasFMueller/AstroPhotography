/*
 * TransformBuilder.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "TransformBuilder.h"
#include <AstroDebug.h>

namespace astro {
namespace image {
namespace transform {

void	TransformBuilder::showResiduals(const Transform& t, 
		const std::vector<Point>& from,
		const std::vector<Point>& to) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "verifying residuals for %s",
		t.toString().c_str());
	double	residual = 0.;
	auto	fromptr = from.begin();
	auto	toptr = to.begin();
	int	i = 0;
	while (fromptr != from.end()) {
		Point	target = t(*fromptr);
		double	delta = distance(target, *toptr);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "residual[%d] = %f",
			i++, delta);
		residual += delta;
		fromptr++;
		toptr++;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "residual = %f", residual);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "average residual %f",
		residual / from.size());
}

} // namespace transform
} // namespace image
} // namespace astro
