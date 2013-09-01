/*
 * Viewer.cpp -- Implementation of a viewer class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroViewer.h>
#include <AstroImage.h>
#include <AstroIO.h>
#include <AstroDebug.h>

using namespace astro::image;
using namespace astro::io;

namespace astro {
namespace image {

template<typename P>
void	convert_mono(Image<RGB<float> >& image, const ImagePtr& rawimage) {
	Image<P>	*imagep = dynamic_cast<Image<P> *>(&*rawimage);
	if (NULL == imagep) {
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "pixel size: %d", sizeof(P));
	unsigned int	width = image.size().width();
	unsigned int	height = image.size().height();
	for (unsigned int x = 0; x < width; x++) {
		for (unsigned int y = 0; y < height; y++) {
			P	v = imagep->pixel(x, y);
			image.pixel(x, y) = RGB<float>(v, v, v);
		}
	}
}

template<typename P>
void	convert_rgb(Image<RGB<float> >& image, const ImagePtr rawimage) {
	Image<RGB<P> >	*imagep = dynamic_cast<Image<RGB<P> > *>(&*rawimage);
	if (NULL == imagep) {
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "RGB pixel size: %d", sizeof(RGB<P>));
	unsigned int	width = image.size().width();
	unsigned int	height = image.size().height();
	for (unsigned int x = 0; x < width; x++) {
		for (unsigned int y = 0; y < height; y++) {
			RGB<P>	v = imagep->pixel(x, y);
			image.pixel(x, y) = RGB<float>(v.R, v.G, v.B);
		}
	}
}

Viewer::Viewer(const std::string& filename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create viewer for file %s",
		filename.c_str());
	// read the FITS image
	FITSin	in(filename);
	ImagePtr	rawimage = in.read();

	// image size
	ImageSize	size = rawimage->size();

	// allocate an image with float pixels
	Image<RGB<float> >	*imagep = new Image<RGB<float> >(size);
	image = ImagePtr(imagep);

	// create an array 
	uint32_t	*p = new uint32_t[size.getPixels()];
	_imagedata = imagedataptr(p);
	
	// copy the data from the image to the imagep
	convert_mono<unsigned char>(*imagep, rawimage);
	convert_mono<unsigned short>(*imagep, rawimage);
	convert_mono<unsigned int>(*imagep, rawimage);
	convert_mono<unsigned long>(*imagep, rawimage);
	convert_mono<float>(*imagep, rawimage);
	convert_mono<double>(*imagep, rawimage);

	// copy the data from the image to the imagep
	convert_rgb<unsigned char>(*imagep, rawimage);
	convert_rgb<unsigned short>(*imagep, rawimage);
	convert_rgb<unsigned int>(*imagep, rawimage);
	convert_rgb<unsigned long>(*imagep, rawimage);
	convert_rgb<float>(*imagep, rawimage);
	convert_rgb<double>(*imagep, rawimage);

	// copy the data to the 
	update();
}

Viewer::~Viewer() {
}

unsigned char	reduce(float value) {
	if (value > 255) {
		value = 255;
	}
	if (value < 0) {
		value = 255;
	}
	unsigned char	result = round(value);
	return result;
}

void	Viewer::update() {
	uint32_t	*p = imagedata();
	if (NULL == p) {
		return;
	}
	ImageSize	size = image->size();
	unsigned int	width = size.width();
	unsigned int	height = size.height();
	Image<RGB<float> >	*imagep = dynamic_cast<Image<RGB<float> >*>(&*image);
	for (unsigned int x = 0; x < width; x++) {
		for (unsigned int y = 0; y < height; y++) {
			unsigned int	offset = size.offset(x, y);
			unsigned char	v = reduce(imagep->pixel(x, y).R / 256.);
			uint32_t	value = v;
			value <<= 8;
			v = reduce(imagep->pixel(x, y).G / 256.);
			value |= v;
			value <<= 8;
			v = reduce(imagep->pixel(x, y).B / 256.);
			value |= v;
			p[offset] = value;
		}
	}
}

uint32_t	*Viewer::imagedata() const {
	return &*_imagedata;
}

ImageSize	Viewer::size() const {
	return image->size();
}

} // namespace image
} // namespace astro
