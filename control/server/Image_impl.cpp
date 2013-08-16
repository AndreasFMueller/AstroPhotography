/*
 * Image_impl.cpp -- CORBA Image wrapper implementation
 * 
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Image_impl.h"

#include <AstroIO.h>

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

void	Image_impl::write(const char *filename, bool overwrite) {
	if (!_image) {
		// XXX bad things should happen if there is no image
		return;
	}
	std::string	f(filename);
	astro::io::FITSout	out(f);
	out.setPrecious(!overwrite);
	out.write(_image);
}

} // namespace astro
