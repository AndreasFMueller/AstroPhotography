/*
 * TransformAnalzer.cpp -- TransformAnalyzer implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTransform.h>
#include <AstroAdapter.h>

using namespace astro::image;

namespace astro {
namespace image {
namespace transform {

/**
 * \brief Get the transform from the base image to the argument image
 */
Transform	TransformAnalyzer::operator()(
			const ConstImageAdapter<double>& image) const {
	// compute a suiteable grid of points where we want to phase
	// correlate
	int	hsteps = ((image.getSize().width() - spacing) / 2) / spacing;
	int	vsteps = ((image.getSize().height() - spacing) / 2) / spacing;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "hsteps = %d, vsteps = %d",
		hsteps, vsteps);
	ImagePoint	center(image.getSize().width() / 2,
				image.getSize().height() / 2);
	std::vector<Point>	frompoints;
	for (int h = -hsteps; h <= hsteps; h++) {
		for (int v = -vsteps; v <= vsteps; v++) {
			Point	point(center.x() + (h * spacing),
					center.y() + (v * spacing));
			frompoints.push_back(point);
		}
	}

	// compute points and translation at these grid points
	std::vector<Point>	topoints;
	std::vector<Point>::const_iterator	fromi;
	PhaseCorrelator	pc;
	for (fromi = frompoints.begin(); fromi != frompoints.end(); fromi++) {
		// compute the window around the from point
		Point	frompoint = *fromi;
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
		Point	topoint = frompoint + translation;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s -> %s",
			frompoint.toString().c_str(),
			topoint.toString().c_str());
		topoints.push_back(topoint);
	}

	// compute the transform from the two sets of points
	return Transform(frompoints, topoints);
}

} // namespace transform
} // namepace image
} // namespace astro
