/*
 * Image_impl.cpp -- CORBA Image wrapper implementation
 * 
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Image_impl.h"

#include <AstroIO.h>
#include <AstroFilterfunc.h>

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

char	*Image_impl::write(const char *filename, bool overwrite) {
	std::string	f(filename);
	try {
		astro::io::FITSout	out(f);
		out.setPrecious(!overwrite);
		out.write(_image);
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot write image file: %s",
			x.what());
		IOException	ioexception;
		ioexception.cause = x.what();
		throw ioexception;
	}
	char	cwd[1024];
	if (NULL == getcwd(cwd, sizeof(cwd))) {
		debug(LOG_ERR, DEBUG_LOG, 0, "CWD not found");
		NotFound	notfound;
		notfound.cause = (const char *)"current working directory not found";
		throw notfound;
	}
	std::string	url = astro::stringprintf("file://%s/%s", cwd, filename);
	return CORBA::string_dup(url.c_str());
}

CORBA::Long	Image_impl::bytesPerPixel() {
	return astro::image::filter::bytesperpixel(_image);
}

CORBA::Long	Image_impl::planes() {
	return astro::image::filter::planes(_image);
}

CORBA::Long	Image_impl::bytesPerValue() {
	return astro::image::filter::bytespervalue(_image);
}

#define	sequence_mono(pixel, size, _image, result)			\
{									\
	astro::image::Image<pixel>	*imagep				\
		= dynamic_cast<astro::image::Image<pixel> *>(&*_image);	\
	if (NULL != imagep) {						\
		for (unsigned int off = 0; off < size; off++) {		\
			(*result)[off] = (*imagep)[off];		\
		}							\
	}								\
}

#define sequence_yuyv(pixel, size, _image, result)			\
{									\
	astro::image::Image<YUYV<pixel> >	*imagep			\
		= dynamic_cast<astro::image::Image<YUYV<pixel> > *>(&*_image);\
	if (NULL != imagep) {						\
		for (unsigned int off = 0; off < size; off++) {		\
			(*result)[2 * off    ] = (*imagep)[off].y;	\
			(*result)[2 * off + 1] = (*imagep)[off].uv;	\
		}							\
	}								\
}

#define sequence_rgb(pixel, size, _image, result)			\
{									\
	astro::image::Image<RGB<pixel> >	*imagep			\
		= dynamic_cast<astro::image::Image<RGB<pixel> > *>(&*_image);\
	if (NULL != imagep) {						\
		for (unsigned int off = 0; off < size; off++) {		\
			(*result)[3 * off    ] = (*imagep)[off].R;	\
			(*result)[3 * off + 1] = (*imagep)[off].G;	\
			(*result)[3 * off + 2] = (*imagep)[off].B;	\
		}							\
	}								\
}

Astro::ByteImage::ByteSequence	*ByteImage_impl::getBytes() {
	Astro::ByteImage::ByteSequence	*result
		= new Astro::ByteImage::ByteSequence();
	unsigned int	size = _image->size().getPixels();
	size_t	bytes = astro::image::filter::planes(_image) * size;
	result->length(bytes);
	sequence_mono(unsigned char, size, _image, result);
	sequence_yuyv(unsigned char, size, _image, result);
	sequence_rgb(unsigned char, size, _image, result);
	return result;
}

Astro::ShortImage::ShortSequence	*ShortImage_impl::getShorts() {
	Astro::ShortImage::ShortSequence	*result
		= new Astro::ShortImage::ShortSequence();
	unsigned int	size = _image->size().getPixels();
	size_t	shorts = astro::image::filter::planes(_image) * size;
	result->length(shorts);
	sequence_mono(unsigned short, size, _image, result);
	sequence_yuyv(unsigned short, size, _image, result);
	sequence_rgb(unsigned short, size, _image, result);
	return result;
}

} // namespace Astro
