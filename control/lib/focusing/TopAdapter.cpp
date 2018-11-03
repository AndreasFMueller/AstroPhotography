/*
 * TopAdapter.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "TopAdapter.h"
#include <AstroFilter.h>

namespace astro {
namespace focusing {

TopAdapter::TopAdapter(FocusableImage imageptr, float top)
	: ConstImageAdapter<float>(imageptr->getSize()), _imageptr(imageptr),
	  _image(*imageptr), _top(top)  {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start with top=%f", top);
	astro::image::filter::Mean<float,float>		mean;
	_top = mean(*imageptr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "final top: %f", _top);
}

float	TopAdapter::pixel(int x, int y) const {
	float	v = _image.pixel(x, y);
	if (v > _top) {
		return _top;
	}
	return v;
}


} // namespace focusing
} // namespace astro
