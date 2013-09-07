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
#include <AstroHistogram.h>

using namespace astro::image;
using namespace astro::adapter;
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
			float	v = imagep->pixel(x, y);
			image.pixel(x, y) = RGB<float>(1., 1., 1.) * v;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "conversion complete");
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "conversion complete");
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

	// copy the data from the image to the luminanceimagep, *imagep
	convert_rgb<unsigned char>(*imagep, rawimage);
	convert_rgb<unsigned short>(*imagep, rawimage);
	convert_rgb<unsigned int>(*imagep, rawimage);
	convert_rgb<unsigned long>(*imagep, rawimage);
	convert_rgb<float>(*imagep, rawimage);
	convert_rgb<double>(*imagep, rawimage);

	// background stuff
	_backgroundsubtract = true;
	_background = BackgroundExtractor(100)(*imagep);

	// color correction and background
	_colorcorrection = RGB<float>(0.7, 1., 1.1);
	_gamma = 1.;
	_min = 0;
	_max = 65535;
	_max *= 10;
	_max = 10000;

	// Histogram
	debug(LOG_DEBUG, DEBUG_LOG, 0, "computing histogram");
	HistogramLinearScale	scale(0, 128000, 256);
	HistogramFactory	factory(scale);

	HistogramPtr	histogram = factory(image, HistogramFactory::HISTOGRAM_LUMINANCE);
	std::string	hs = histogram->toString();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "histogram: %s", hs.c_str());

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

void	Viewer::writeimage(const std::string& filename) {
	ImageSize	size = image->size();
	Image<RGB<unsigned char> >	*outimage
		= new Image<RGB<unsigned char> >(size);
	unsigned int	width = size.width();
	unsigned int	height = size.height();
	uint32_t	*i = imagedata();
	for (unsigned int x = 0; x < width; x++) {
		for (unsigned int y = 0; y < height; y++) {
			uint32_t	v = i[size.offset(x, y)];
			unsigned char	R = (v & 0xff0000) >> 16;
			unsigned char	G = (v & 0x00ff00) >> 8;
			unsigned char	B = (v & 0x0000ff);
			//debug(LOG_DEBUG, DEBUG_LOG, 0, "%d,%d,%d", R, G, B);
			outimage->pixel(x, y) = RGB<unsigned char>(R, G, B);
		}
	}
	FITSout	out(filename);
	out.setPrecious(false);
	out.write(ImagePtr(outimage));
}

void	Viewer::update() {
	uint32_t	*p = imagedata();
	if (NULL == p) {
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "updating image data at %p", p);
	ImageSize	size = image->size();
	unsigned int	width = size.width();
	unsigned int	height = size.height();
	Image<RGB<float> >	*imagep
		= dynamic_cast<Image<RGB<float> >*>(&*image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "imagep = %p", imagep);

	// compose the processing pipeline
	// step 1: background subtraction
	BackgroundSubtractionAdapter	bsa(*imagep, _background);
	IdentityAdapter<RGB<float> >	identity(*imagep);
	ConstImageAdapter<RGB<float> >	*w = NULL;
	if (_backgroundsubtract) {
		w = &bsa;
	} else {
		w = &identity;
	}

	// step 2: color correction
	ColorCorrectionAdapter<float>	cca(*w, _colorcorrection);

	// for the following steps, we need luminance and color separately
	LuminanceExtractionAdapter<float>	luminanceimage(cca);
	ColorExtractionAdapter<float>	colorimage(cca);

	// step 3: range reduction
	RangeAdapter<float>	range(luminanceimage, _min, _max);

	// step 4: gamma correction
	GammaAdapter<float>	ga(range, _gamma);

	// rescale to the range 0-256
	LuminanceScalingAdapter<float>	upscale(ga, 256);
	LuminanceColorAdapter<float>	lca(upscale, colorimage);

	// RGB32 extractor
	RGB32Adapter<float>	rgb32(lca);

	// extract the data
	debug(LOG_DEBUG, DEBUG_LOG, 0, "extracting RGB32 image");
	for (unsigned int x = 0; x < width; x++) {
		for (unsigned int y = 0; y < height; y++) {
			uint32_t	value = rgb32.pixel(x, y);
			p[size.offset(x, y)] = value;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "update complete");

	// write the file
	writeimage("out.fits");
}

uint32_t	*Viewer::imagedata() const {
	return &*_imagedata;
}

ImageSize	Viewer::size() const {
	return image->size();
}

} // namespace image
} // namespace astro
