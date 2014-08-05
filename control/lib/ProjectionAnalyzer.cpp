/*
 * ProjectionAnalyzer.cpp -- Analyze residuals of a projection
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProjection.h>
#include <AstroAdapter.h>

using namespace astro::adapter;

namespace astro {
namespace image {
namespace project {

std::vector<Residual>	ProjectionAnalyzer::operator()(const ConstImageAdapter<double>& image) const {
	std::vector<Residual>	result;

	// compute a suiteable grid of points where we want to phase
	// correlate
	int	hsteps = ((image.getSize().width() - spacing) / 2) / spacing;
	int	vsteps = ((image.getSize().height() - spacing) / 2) / spacing;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "hsteps = %d, vsteps = %d",
		hsteps, vsteps);
	ImagePoint	center(image.getSize().width() / 2,
				image.getSize().height() / 2);

	// to detect the shifts, we use a phase correlator
	transform::PhaseCorrelator	pc;

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
			WindowAdapter<double>	frompatch(baseimage, window);
			WindowAdapter<double>	topatch(image, window);
			Point	translation = pc(frompatch, topatch);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%s -> %s",
				frompoint.toString().c_str(),
				translation.toString().c_str());

			// add the residual to the result set
			result.push_back(Residual(frompoint, translation));
		}
	}

	return result;
}

} // namespace project
} // namespace image 
} // namespace astro
