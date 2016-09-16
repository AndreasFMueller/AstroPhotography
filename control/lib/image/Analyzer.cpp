/*
 * Analyzer.cpp -- Analyze residuals of a transform
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTransform.h>
#include <AstroAdapter.h>
#include <AstroFormat.h>
#include <AstroDebug.h>

using namespace astro::adapter;

namespace astro {
namespace image {
namespace transform {

Analyzer::Analyzer(const ConstImageAdapter<double>& baseimage,
	int spacing, int patchsize)
	: _baseimage(baseimage), _spacing(spacing), _patchsize(patchsize)  {
	if (_spacing < 0) {
		std::string	msg = stringprintf("invalid spacing %d",
			_spacing);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::range_error(msg);
	}
	if (_patchsize < 0) {
		std::string	msg = stringprintf("invalid patchsize %d",
			_patchsize);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::range_error(msg);
	}
	_hanning = false;
}


std::vector<Residual>	Analyzer::operator()(const ConstImageAdapter<double>& image) const {
	// first find out whether the patch size fits inside the image
	if ((_patchsize > image.getSize().width())
		|| (_patchsize > image.getSize().height())) {
		std::string	msg = stringprintf("patch size %d does not fit into image", _patchsize);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// build a set of patches
	ImageSize	size = image.getSize();
	int	hsteps = (size.width() - _patchsize) / _spacing;
	int	xoffset = (size.width() - hsteps * _spacing) / 2;
	int	vsteps = (size.height() - _patchsize) / _spacing;
	int	yoffset = (size.height() - vsteps * _spacing) / 2;
	std::vector<ImagePoint>	points;
	for (int h = 0; h <= hsteps; h++) {
		for (int v = 0; v <= vsteps; v++) {
			ImagePoint	point(xoffset + h * _spacing,
						yoffset + v * _spacing);
			points.push_back(point);
		}
	}

	// now compute the shift for each point
	std::vector<Residual>	result;
	for (auto pt = points.begin(); pt != points.end(); pt++) {
		try {
			Residual	residual = translation(image, *pt,
							_patchsize);
			if (residual.valid()) {
				result.push_back(residual);
			}
		} catch (const std::exception& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "ignoring point %s: %s",
				pt->toString().c_str(), x.what());
		}
	}

	// display resulting residuals if in debug mode
	if (debuglevel >= LOG_DEBUG) {
		for (std::vector<Residual>::size_type i = 0; i < result.size();
			i++) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "Residual[%d] %s", i,
				std::string(result[i]).c_str());
		}
	}

	return result;
}

Residual	Analyzer::translation(const ConstImageAdapter<double>&image,
	const ImagePoint& where, int patchsize) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get translation at %s",
		std::string(where).c_str());
	// create the subwindow we want to lock at
	int	xoffset = where.x() - patchsize / 2;
	int	yoffset = where.y() - patchsize / 2;
	ImagePoint	patchcorner(xoffset, yoffset);
	ImageRectangle	window(patchcorner, ImageSize(patchsize, patchsize));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "window: %s",
		window.toString().c_str());

	// we need a phase correlator to measure the transform
	transform::PhaseCorrelator	pc(_hanning);

	// compute the translation between the windows
	WindowAdapter<double>	frompatch(image, window);
	WindowAdapter<double>	topatch(_baseimage, window);
	std::pair<Point, double>	delta = pc(frompatch, topatch);
	Point	translation = delta.first;
	double	weight = delta.second;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s -> %s", where.toString().c_str(),
		translation.toString().c_str());

	// add the residual to the result set
	Residual	residual(where, translation, weight);
	return residual;
}

} // namespace transform
} // namespace image 
} // namespace astro
