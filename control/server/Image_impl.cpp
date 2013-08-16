/*
 * Image_impl.cpp -- CORBA Image wrapper implementation
 * 
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Image_impl.h"

namespace Astro {

ImagePoint	Image_impl::origin() {
	astro::image::ImagePoint	o = _image->origin();
	ImagePoint	result;
	result.x = o.x();
	result.y = o.y();
	return result;
}

ImageSize	Image_impl::size() {
	astro::image::ImageSize	s = _image->size();
	ImageSize	result;
	result.width = s.width();
	result.height = s.height();
	return result;
}

} // namespace astro
