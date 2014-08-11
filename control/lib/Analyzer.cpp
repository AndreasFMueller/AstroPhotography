/*
 * Analyzer.cpp -- Analyze residuals of a transform
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTransform.h>
#include <AstroAdapter.h>

using namespace astro::adapter;

namespace astro {
namespace image {
namespace transform {

std::vector<Residual>	Analyzer::operator()(const ConstImageAdapter<double>& image) const {
	std::vector<Residual>	result;

	// compute a suiteable grid of points where we want to phase
	// correlate
	int	hsteps = ((image.getSize().width() - patchsize) / 2) / spacing;
	int	vsteps = ((image.getSize().height() - patchsize) / 2) / spacing;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "hsteps = %d, vsteps = %d",
		hsteps, vsteps);
	ImagePoint	center = image.getSize().center();

	// to detect the shifts, we use a phase correlator
	transform::PhaseCorrelator	pc(false);

	// now compute the shift for each point
	for (int h = -hsteps; h <= hsteps; h++) {
		for (int v = -vsteps; v <= vsteps; v++) {
			ImagePoint	frompoint(center.x() + (h * spacing),
						center.y() + (v * spacing));

			int	xoffset = frompoint.x() - patchsize / 2;
			int	yoffset = frompoint.y() - patchsize / 2;
			ImagePoint	patchcorner(xoffset, yoffset);
			ImageRectangle	window(patchcorner,
					ImageSize(patchsize, patchsize));
			debug(LOG_DEBUG, DEBUG_LOG, 0, "window: %s",
				window.toString().c_str());

			// compute the translation between the windows
			WindowAdapter<double>	frompatch(image, window);
			WindowAdapter<double>	topatch(baseimage, window);
			std::pair<Point, double>	delta
				= pc(frompatch, topatch);
			Point	translation = delta.first;
			double	weight = delta.second;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%s -> %s",
				frompoint.toString().c_str(),
				translation.toString().c_str());

			// add the residual to the result set
			Residual	residual(frompoint, translation,
						weight);
			if (residual.valid()) {
				result.push_back(residual);
			}
		}
	}

	return result;
}

} // namespace transform
} // namespace image 
} // namespace astro
