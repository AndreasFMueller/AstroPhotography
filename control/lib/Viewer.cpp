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
#include <AstroFilter.h>
#include <ViewerPipeline.h>

using namespace astro::image;
using namespace astro::adapter;
using namespace astro::io;

namespace astro {
namespace image {

/**
 * \brief Convert an monochrome image to RGB float pixels
 */
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

/**
 * \brief Convert an RGB image to RGB float pixels
 */
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

/**
 * \brief Create a new Viewer
 *
 * This constructor converts the image to an RGB<float> image, and
 * then sets up the image processing pipeline on this image.
 * For the pipeline, some objects have to computed, like the white
 * balance, the background gradients.
 */
Viewer::Viewer(const std::string& filename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create viewer for file %s",
		filename.c_str());
	// read the FITS image
	FITSin	in(filename);
	ImagePtr	rawimage = in.read();

	// image size
	ImageSize	size = rawimage->size();
	_displaysize = size;

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

	// create the viewer pipeline
	pipeline = new ViewerPipeline(imagep);
	pipelineptr = std::tr1::shared_ptr<ViewerPipeline>(pipeline);

	// compute the white balance vector
	filter::WhiteBalance<float>	wb;
	RGB<double>	rgb = wb.filter(*imagep);
	rgb = rgb.inverse().normalize();

	// background stuff
	Background<float>	bg = BackgroundExtractor(100)(*imagep);
	background(bg);
	backgroundEnabled(true);
	gradientEnabled(true);

	// set parameters
	pipeline->colorcorrection(RGB<float>(rgb.R, rgb.G, rgb.B));
	pipeline->setRange(0, 10000);

	// Histogram
	debug(LOG_DEBUG, DEBUG_LOG, 0, "computing histogram");
	_histograms = HistogramSet(image, 350);

	// set up the preview size
	previewwidth(300);

	// set up the background preview
	debug(LOG_DEBUG, DEBUG_LOG, 0, "computing background size");
	unsigned int	width = 100;
	unsigned int	height = (size.height() * width) / size.width();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "background %u x %u", width, height);
	backgroundsize(ImageSize(width, height));

	// copy the data to the 
	update();
	previewupdate();
}

Viewer::~Viewer() {
}

/**
 * \brief Write a processed image to a file
 */
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

RGB<float>	Viewer::colorcorrection() const {
	return pipeline->colorcorrection();
}

void	Viewer::colorcorrection(const RGB<float>& colorcorrection) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "color correction: [%.2f, %.2f, %.2f]",
		colorcorrection.R, colorcorrection.G, colorcorrection.B);
	pipeline->colorcorrection(colorcorrection);
}

float	Viewer::min() const {
	return pipeline->min();
}

float	Viewer::max() const {
	return pipeline->max();
}

void	Viewer::setRange(float min, float max) {
	pipeline->setRange(min, max);
}

float	Viewer::gamma() const {
	return pipeline->gamma();
}

void	Viewer::gamma(float _gamma) {
	pipeline->gamma(_gamma);
}

float	Viewer::saturation() const {
	return pipeline->saturation();
}

void	Viewer::saturation(float _saturation) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "saturation set to %.3f", _saturation);
	pipeline->saturation(_saturation);
}

const ImageSize&	Viewer::displaysize() const {
	return _displaysize;
}

void	Viewer::displaysize(const ImageSize& displaysize) {
	_displaysize = displaysize;
}

void	Viewer::displayScale(float scale) {
	if (scale > 1.0) {
		throw std::range_error("cannot scale up");
	}
	if (scale < 0) {
		throw std::range_error("negative scale not allowed");
	}
	_displaysize = size() * scale;
}

double	Viewer::displayScale() const {
	return _displaysize.width() / (double)size().width();
}

const Background<float>&	Viewer::background() const {
	return pipeline->background();
}

void	Viewer::background(const Background<float>& _background) {
	return pipeline->background(_background);
}

bool	Viewer::backgroundEnabled() const {
	return pipeline->backgroundEnabled();
}

void	Viewer::backgroundEnabled(bool _backgroundenabled) {
	pipeline->backgroundEnabled(_backgroundenabled);
}

bool	Viewer::gradientEnabled() const {
	return pipeline->gradientEnabled();
}

void	Viewer::gradientEnabled(bool _gradientenabled) {
	return pipeline->gradientEnabled(_gradientenabled);
}

void	Viewer::previewsize(const ImageSize& previewsize) {
	_previewsize = previewsize;
	uint32_t	*p = new uint32_t[_previewsize.getPixels()];
	_previewdata = imagedataptr(p);
}

void	Viewer::previewwidth(unsigned int width) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set width to %u", width);
	ImageSize	size = image->size();
	unsigned int	height = (size.height() * width) / size.width();
	previewsize(ImageSize(width, height));
}

void	Viewer::backgroundsize(const ImageSize& backgroundsize) {
	_backgroundsize = backgroundsize;
	uint32_t	*p = new uint32_t[_previewsize.getPixels()];
	_backgrounddata = imagedataptr(p);
}

/**
 * \brief Update the preview pixel array
 */
void	Viewer::previewupdate() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "preview update");
	uint32_t	*p = previewdata();
	if (NULL == p) {
		return;
	}
	unsigned int	width = _previewsize.width();
	unsigned int	height = _previewsize.height();

	// create a subgrid adaptero
	ImageRectangle	rectangle(size());
	WindowScalingAdapter<unsigned int>	wsa(*pipeline,
		rectangle, _previewsize);

	for (unsigned int x = 0; x < width; x++) {
		for (unsigned int y = 0; y < height; y++) {
			uint32_t	value = wsa.pixel(x, y);
			p[_previewsize.offset(x, y)] = value;
		}
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "preview update complete");
}

/**
 * \brief Update the background pixel array
 */
void	Viewer::backgroundupdate() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "backgroundupdate");
	uint32_t	*p = imagedata();
	if (NULL == p) {
		return;
	}
	unsigned int	width = backgroundsize().width();
	unsigned int	height = backgroundsize().height();

	BackgroundImageAdapter<float, unsigned char>	bia(backgroundsize(),
								background());
	RGB32Adapter<unsigned char>	rgb32(bia);
	for (unsigned int x = 0; x < width; x++) {
		for (unsigned int y = 0; y < height; y++) {
			p[backgroundsize().offset(x, y)] = rgb32.pixel(x, y);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "backgroundupdate complete");
}

/**
 * \brief Update the current pixel arrays
 */
void	Viewer::update() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "main update");
	uint32_t	*p = imagedata();
	if ((NULL == p) || (NULL == pipeline)) {
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "updating image data at %p", p);
	unsigned int	width = _displaysize.width();
	unsigned int	height = _displaysize.height();

	// extract the image
	ImageRectangle	rectangle(size());
	WindowScalingAdapter<unsigned int>	wsa(*pipeline, rectangle,
							_displaysize);

	// extract the data
	debug(LOG_DEBUG, DEBUG_LOG, 0, "extracting %u x %u RGB32 image",
		width, height);
	for (unsigned int x = 0; x < width; x++) {
		for (unsigned int y = 0; y < height; y++) {
			p[_displaysize.offset(x, y)] = wsa.pixel(x, y);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "main update complete");
}

uint32_t	*Viewer::imagedata() const {
	return &*_imagedata;
}

uint32_t	*Viewer::previewdata() const {
	return &*_previewdata;
}

uint32_t	*Viewer::backgrounddata() const {
	return &*_backgrounddata;
}

const ImageSize&	Viewer::size() const {
	return image->size();
}

} // namespace image
} // namespace astro
