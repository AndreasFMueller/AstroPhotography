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
	// first find out whether the patch size fits inside the image
	if ((patchsize > image.getSize().width())
		|| (patchsize > image.getSize().height())) {
		throw std::runtime_error("patch size does not fit into image");
	}

	// build a set of patches
	ImageSize	size = image.getSize();
	int	hsteps = (size.width() - patchsize) / spacing;
	int	xoffset = (size.width() - hsteps * spacing) / 2;
	int	vsteps = (size.height() - patchsize) / spacing;
	int	yoffset = (size.height() - vsteps * spacing) / 2;
	std::vector<ImagePoint>	points;
	for (int h = 0; h <= hsteps; h++) {
		for (int v = 0; v <= vsteps; v++) {
			ImagePoint	point(xoffset + h * spacing,
						yoffset + v * spacing);
			points.push_back(point);
		}
	}

	// now compute the shift for each point
	std::vector<Residual>	result;
	for (auto pt = points.begin(); pt != points.end(); pt++) {
		Residual	residual = translation(image, *pt, patchsize);
		if (residual.valid()) {
			result.push_back(residual);
		}
	}

	// display resulting residuals if in debug mode
	if (debuglevel >= LOG_DEBUG) {
		for (std::vector<Residual>::size_type i = 0; i < result.size();
			i++) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "residual[%d] %s", i,
				std::string(result[i]).c_str());
		}
	}

	return result;
}

Residual	Analyzer::translation(const ConstImageAdapter<double>&image,
	const ImagePoint& where, unsigned int _patchsize) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get translation at %s",
		std::string(where).c_str());
	// create the subwindow we want to lock at
	int	xoffset = where.x() - _patchsize / 2;
	int	yoffset = where.y() - _patchsize / 2;
	ImagePoint	patchcorner(xoffset, yoffset);
	ImageRectangle	window(patchcorner,
			ImageSize(patchsize, _patchsize));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "window: %s",
		window.toString().c_str());

	// we need a phase correlator to measure the transform
	transform::PhaseCorrelator	pc(false);

	// compute the translation between the windows
	WindowAdapter<double>	frompatch(image, window);
	WindowAdapter<double>	topatch(baseimage, window);
	std::pair<Point, double>	delta
		= pc(frompatch, topatch);
	Point	translation = delta.first;
	double	weight = delta.second;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s -> %s",
		where.toString().c_str(),
		translation.toString().c_str());

	// add the residual to the result set
	Residual	residual(where, translation, weight);
	return residual;
}

} // namespace transform
} // namespace image 
} // namespace astro
