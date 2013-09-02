/*
 * Viewer.cpp -- Implementation of a viewer class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroViewer.h>
#include <AstroImage.h>
#include <AstroIO.h>
#include <AstroDebug.h>
#include <AstroTonemapping.h>

using namespace astro::image;
using namespace astro::adapter;
using namespace astro::io;

namespace astro {
namespace image {

template<typename P>
void	convert_mono(Image<float>& luminanceimage,
		Image<RGB<float> >& colorimage, const ImagePtr& rawimage) {
	Image<P>	*imagep = dynamic_cast<Image<P> *>(&*rawimage);
	if (NULL == imagep) {
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "pixel size: %d", sizeof(P));
	unsigned int	width = luminanceimage.size().width();
	unsigned int	height = luminanceimage.size().height();
	for (unsigned int x = 0; x < width; x++) {
		for (unsigned int y = 0; y < height; y++) {
			P	v = imagep->pixel(x, y);
			luminanceimage.pixel(x, y) = v; 
			colorimage.pixel(x, y) = RGB<float>(1., 1., 1.);
		}
	}
}

template<typename P>
void	convert_rgb(Image<float>& luminanceimage,
		Image<RGB<float> >& colorimage, const ImagePtr rawimage) {
	Image<RGB<P> >	*imagep = dynamic_cast<Image<RGB<P> > *>(&*rawimage);
	if (NULL == imagep) {
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "RGB pixel size: %d", sizeof(RGB<P>));
	unsigned int	width = luminanceimage.size().width();
	unsigned int	height = luminanceimage.size().height();
	for (unsigned int x = 0; x < width; x++) {
		for (unsigned int y = 0; y < height; y++) {
			RGB<P>	v = imagep->pixel(x, y);
			float	l = v.luminance();
			luminanceimage.pixel(x, y) = l;
			colorimage.pixel(x, y) = RGB<float>(v.R, v.G, v.B) / l;
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
	Image<RGB<float> >	*colorimagep = new Image<RGB<float> >(size);
	colorimage = ImagePtr(colorimagep);
	Image<float>	*luminanceimagep = new Image<float>(size);
	luminanceimage = ImagePtr(luminanceimagep);

	// create an array 
	uint32_t	*p = new uint32_t[size.getPixels()];
	_imagedata = imagedataptr(p);
	
	// copy the data from the image to the imagep
	convert_mono<unsigned char>(*luminanceimagep, *colorimagep, rawimage);
	convert_mono<unsigned short>(*luminanceimagep, *colorimagep, rawimage);
	convert_mono<unsigned int>(*luminanceimagep, *colorimagep, rawimage);
	convert_mono<unsigned long>(*luminanceimagep, *colorimagep, rawimage);
	convert_mono<float>(*luminanceimagep, *colorimagep, rawimage);
	convert_mono<double>(*luminanceimagep, *colorimagep, rawimage);

	// copy the data from the image to the luminanceimagep, *colorimagep
	convert_rgb<unsigned char>(*luminanceimagep, *colorimagep, rawimage);
	convert_rgb<unsigned short>(*luminanceimagep, *colorimagep, rawimage);
	convert_rgb<unsigned int>(*luminanceimagep, *colorimagep, rawimage);
	convert_rgb<unsigned long>(*luminanceimagep, *colorimagep, rawimage);
	convert_rgb<float>(*luminanceimagep, *colorimagep, rawimage);
	convert_rgb<double>(*luminanceimagep, *colorimagep, rawimage);

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

uint32_t	reduce(const RGB<float> pixel) {
	uint32_t	result;
	result = reduce(pixel.R);
	result <<= 8;
	result |= reduce(pixel.G);
	result <<= 8;
	result |= reduce(pixel.B);
	return result;
}

void	Viewer::update() {
	uint32_t	*p = imagedata();
	if (NULL == p) {
		return;
	}
	ImageSize	size = luminanceimage->size();
	unsigned int	width = size.width();
	unsigned int	height = size.height();
	Image<float>	*luminanceimagep
		= dynamic_cast<Image<float> *>(&*luminanceimage);
	Image<RGB<float> >	*colorimagep
		= dynamic_cast<Image<RGB<float> >*>(&*colorimage);

	// compose the processing pipeline
	LuminanceScalingAdapter<float>	lsa(*luminanceimagep, 1/65535.);
	ColorCorrectionAdapter<float>	cca(*colorimagep, RGB<float>(0.7,1.,1.));
	BackgroundSubtractionAdapter<float>	bsa(cca, RGB<float>(.2, .2, .2));
	GammaAdapter<float>	ga(lsa, 0.5);
	LuminanceScalingAdapter<float>	upscale(ga, 256);
	LuminanceColorAdapter<float>	lca(upscale, bsa);

	for (unsigned int x = 0; x < width; x++) {
		for (unsigned int y = 0; y < height; y++) {
			RGB<float>	colorpixel = lca.pixel(x, y);
			uint32_t	value = reduce(colorpixel);
			unsigned int	offset = size.offset(x, y);
			p[offset] = value;
		}
	}
}

uint32_t	*Viewer::imagedata() const {
	return &*_imagedata;
}

ImageSize	Viewer::size() const {
	return luminanceimage->size();
}

} // namespace image
} // namespace astro
