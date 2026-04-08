/*
 * AberrationInspectorLayout.cpp
 *
 * (c) 2024 Prof Dr Andreas Müller
 */
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroImage.h>
#include <vector>

namespace astro {
namespace image {

/**
 * \brief Normalize to an even number if _mosaic is set
 *
 * \param x	the number to normalize
 */
int	AberrationInspectorLayout::even(int x) const {
	if (_mosaic) 
		return x - (x % 2);
	return x;
}

/**
 * \brief Construkctor for the AberrationInspectorLayout
 *
 * \param targetsize	the size of the target image
 */
AberrationInspectorLayout::AberrationInspectorLayout(const ImageSize& targetsize,
	const ImageSize& sourcesize, bool mosaic) 
	: _targetsize(targetsize), _sourcesize(sourcesize),
	  _mosaic(mosaic) {
}

/**
 * \brief Retrieve a window as a rectangle pair
 *
 * \param i	the number of the window
 */
const AberrationInspectorLayout::windowpair_t&	AberrationInspectorLayout::window(size_t i) const {
	return _windowlist[i];
}

/**
 * \brief compute a list of rectangles for the aberration inspector
 *
 * \param hwindows	number of horizontal windows
 * \param vwindows	number of vertical windows
 * \param gap		gap between the windows
 */
void	AberrationInspectorLayout::layout(int hwindows, int vwindows, int gap) {
	// clear the list of window pairs
	_windowlist.clear();

	// compute target rectangle dimensions
	int	hgapsize = even((hwindows - 1) * gap);
	int	windowwidth = even((_targetsize.width() - hgapsize)
			/ hwindows);
	int	vgapsize = even((vwindows - 1) * gap);
	int	windowheight = even((_targetsize.height() - vgapsize)
			/ vwindows);
	ImageSize	windowsize(windowwidth, windowheight);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "window size is %s",
		windowsize.toString().c_str());

	// make sure that the target rectangle size fits into the source
	// image rectangle
	if (!(_sourcesize >= windowsize)) {
		std::string	msg = astro::stringprintf(
			"source image too small: %s < %s",
			_sourcesize.toString().c_str(),
			windowsize.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// compute the gaps
	hgapsize = even((_targetsize.width() - hwindows * windowwidth)
		/ (hwindows - 1));
	vgapsize = even((_targetsize.height() - vwindows * windowheight)
		/ (vwindows - 1));

	// compute the source offsets
	int	hoffset = even((_sourcesize.width() - windowwidth)
			/ (hwindows - 1));
	int	voffset = even((_sourcesize.height() - windowheight)
			/ (vwindows - 1));

	// compute source/target pairs
	for (int h = 0; h < hwindows; h++) {
		for (int v = 0; v < vwindows; v++) {
			ImageRectangle	_target(ImagePoint(
					h * (windowwidth + hgapsize),
					v * (windowheight + vgapsize)),
				windowsize);
			ImageRectangle	_source(
				ImagePoint(h * hoffset, v * voffset),
				windowsize);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "add pair %s -> %s",
				_source.toString().c_str(),
				_target.toString().c_str());
			_windowlist.push_back(std::make_pair(_source, _target));
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "layout with %d windows created",
		_windowlist.size());
}

} // namespace image
} // namespace astro
