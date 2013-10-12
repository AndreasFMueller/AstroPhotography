/**
 * CalibrationFactory.cpp -- compute calibration frames
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroCalibration.h>
#include <AstroFilter.h>
#include <PixelValue.h>
#include <limits>
#include <AstroDebug.h>
#include <stdexcept>
#include <vector>
#include <AstroFormat.h>
#include <AstroIO.h>

using namespace astro::image;
using namespace astro::image::filter;
using namespace astro::camera;
using namespace astro::adapter;

namespace astro {
namespace calibration {

/**
 * \brief Check that the image sequence is consistent 
 *
 * Only a if all the images are of the same size we can actually compute
 * a calibration image.
 * \param images
 */
bool	consistent(const ImageSequence& images) {
	// make sure all images in the sequence are of the same size
	ImageSequence::const_iterator	i = images.begin();
	for (i++; i != images.end(); i++) {
		if ((*images.begin())->size() != (*i)->size()) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "image size mismatch");
			return false;
		}
	}

	// make sure all the images are monochrome images. There is now way
	// to calibrate color image,
	for (i = images.begin(); i != images.end(); i++) {
		if (isColorImage(*i)) {
			return false;
		}
	}
	return true;
}

/**
 * \brief Factory method
 *
 * This is the factory method, it takes an image sequence and produces
 * a calibration image. The base class of course has no data on which
 * to base the creation of a calibration image, so it just returns an
 * empty image pointer.
 */
ImagePtr	CalibrationFrameFactory::operator()(const ImageSequence& images) const {
	debug(LOG_ERR, DEBUG_LOG, 0, "base class factory method called, "
		"probably an error");
	return ImagePtr();
}

/**
 * \brief Compute statistical characteristics of an image sequence
 *
 * This class is needed by several methods that compute means, variance
 * and medians to decide whether or not to consider an image pixel as valid.
 * It usually operates on a sequence of images, which must all have the same
 * pixel type.
 */
template<typename T>
class ImageMean {
	bool	enableVariance;
	unsigned int	k;
public:
	void	setK(unsigned int _k) { k = _k; }
	typedef ConstPixelValue<T>	PV;
	typedef std::vector<PV>	PVSequence;

private:
	PVSequence	pvs;
	void	setup_pv(const ImageSequence& images);
public:

	ImageSize	size;
	/**
	 * \brief Calibration image being computed
	 *
	 * This image contains the mean values for pixels at the same position
	 */
	Image<T>	*image;

	/**
	 * \brief Variance per pixel
	 *
	 * This image contains the variance of pixel values at the same position
 	 */
	Image<T>	*var;

private:
	void	setup_images(const ImageSequence& images);
	void	compute(unsigned int x, unsigned int y, T darkvalue);

public:
	ImageMean(const ImageSequence& images, bool _enableVariance = false);
	ImageMean(const ImageSequence& images, const Image<T>& dark,
		bool _enableVariance = false);

	~ImageMean();
	T	mean(const Subgrid grid = Subgrid()) const;
	T	variance(const Subgrid grid = Subgrid()) const;
	ImagePtr	getImagePtr();
};

/**
 * \brief Prepare internal data for dark image compuation
 */
template<typename T>
void	ImageMean<T>::setup_images(const ImageSequence& images) {
	// create an image of appropriate size
	size = (*images.begin())->size();
	image = new Image<T>(size);
	if (enableVariance) {
		// prepare the variance image
		var = new Image<T>(size);
	} else {
		var = NULL;
	}
}

/**
 * \brief Prepare internal data
 *
 * This method is called to set up the PixelValue vectors
 */
template<typename T>
void	ImageMean<T>::setup_pv(const ImageSequence& images) {
	// the image sequence must be consistent, or we cannot do 
	// anything about it
	if (!consistent(images)) {
		throw std::runtime_error("images not consistent");
	}

	// we need access to the pixels, but we want to avoid all the
	// time consuming dynamic casts, so we create a vector of
	// PixelValue objects, which already do the dynamic casts
	// in the constructor
	ImageSequence::const_iterator i;
	for (i = images.begin(); i != images.end(); i++) {
		pvs.push_back(PV(*i));
	}
}

/**
 * \brief Perform dark image computation per pixel
 *
 * Computes mean and variance (if enabled) of the pixels
 * at point (x,y) from all images in the image sequence.
 * The PixelValue objects are used for this purpose.
 * \param x	x-coordinate of pixel
 * \param y	y-coordinate of pixel
 */
template<typename T>
void	ImageMean<T>::compute(unsigned int x, unsigned int y, T darkvalue) {
	// if the dark value is invalid, then the computed value
	// is also invalid
	if (darkvalue != darkvalue) {
		image->pixel(x, y) = darkvalue;
		var->pixel(x, y) = darkvalue;
		return;
	}

	// perform mean (and possibly variance) computation in the
	// case where 
	//T	m;
	T	X = 0, X2 = 0;
	typename std::vector<PV>::const_iterator j;
	unsigned int	counter = 0;
	for (j = pvs.begin(); j != pvs.end(); j++) {
		T	v = j->pixelvalue(x, y);
		// skip this value if it is a NaN
		if (v != v)
			continue;
		if (v < darkvalue) {
			v = 0;
		} else {
			v = v - darkvalue;
		}
		X += v;
		if (enableVariance) {
			X2 += v * v;
		}
		counter++;
	}
	if (counter != pvs.size()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "bad pixel values at (%d, %d): %d", x, y, counter);
	}
	T	EX = X / counter;
	T	EX2 = 0;
	if (enableVariance) {
		EX2 = X2 / counter;
	}

	// if we don't have the variance, we leave it at that
	if (!enableVariance) {
		image->pixel(x, y) = EX;
		return;
	}

	// if the variance is enabled, then we can do the computation
	// again, and ignore not only the bad values, but also the
	// ones that are more then 3 standard deviations away from 
	// the mean
	X = 0, X2 = 0;
	counter = 0;
	T	stddevk = k * sqrt(EX2 - EX * EX);
	if (stddevk < 1) {
		stddevk = std::numeric_limits<T>::infinity();
	}
	for (j = pvs.begin(); j != pvs.end(); j++) {
		T	v = j->pixelvalue(x, y);
		// skip NaNs
		if (v != v)
			continue;
		if (v < darkvalue) {
			v = 0;
		} else {
			v = v - darkvalue;
		}
		// skip values that are too far off
		if (fabs(v - EX) > stddevk) {
			continue;
		}
		X += v;
		X2 += v * v;
		counter++;
	}

	if (0 == counter) {
		image->pixel(x, y) = std::numeric_limits<T>::quiet_NaN();
		var->pixel(x, y) = std::numeric_limits<T>::quiet_NaN();
		return;
	}
	EX = X / counter;
	EX2 = X2 / counter;
	image->pixel(x, y) = EX;
	var->pixel(x, y) = EX2 - EX * EX;
}

/**
 * \brief Constructor for ImageMean object
 *
 * The constructor remembers all images, sets up PixelValue objects
 * for them, and computes mean and variance for each point
 * \param images	a sequence of images
 * \param _enableVariance	whether or not the variance should be
 *				computed
 */
template<typename T>
ImageMean<T>::ImageMean(const ImageSequence& images, bool _enableVariance)
		: enableVariance(_enableVariance) {
	// compute the PixelValue objects
	setup_pv(images);

	// allocate the images
	setup_images(images);

	// now compute mean and variance for every pixel
	for (unsigned int x = 0; x < size.width(); x++) {
		for (unsigned int y = 0; y < size.height(); y++) {
			compute(x, y, 0);
		}
	}

	// number of standard deviations for bad pixels
	k = 3;
}

/**
 * \brief Construtor for ImageMean object with dark value correction
 * 
 * Constructs an ImageMean object, but ignores pixels where the
 * dark image has NaN values. This allows to first construct a
 * map of dark pixels, which should be ignored, and then perform
 * the computation of the dark images ignoring the bad pixels.
 */
template<typename T>
ImageMean<T>::ImageMean(const ImageSequence& images, const Image<T>& dark,
	bool _enableVariance) : enableVariance(_enableVariance) {
	// compute the PixelValue objects
	setup_pv(images);

	// allocate the images
	setup_images(images);

	// now compute mean and variance for every pixel
	for (unsigned int x = 0; x < size.width(); x++) {
		for (unsigned int y = 0; y < size.height(); y++) {
			T	darkvalue = dark.pixel(x, y);
			compute(x, y, darkvalue);
		}
	}

	// number of standard deviations for bad pixels
	k = 3;
}

template<typename T>
ImageMean<T>::~ImageMean() {
	if (image) {
		delete image;
		image = NULL;
	}
	if (var) {
		delete var;
		image = NULL;
	}
}

/**
 * \brief compute the mean of the result image
 */
template<typename T>
T	ImageMean<T>::mean(const Subgrid grid) const {
	Mean<T, T>	meanoperator;
	ConstSubgridAdapter<T>	sga(*image, grid);
	return meanoperator(sga);
}

/**
 * \brief compute variance of the result image
 */
template<typename T>
T	ImageMean<T>::variance(const Subgrid grid) const {
	Variance<T, T>	varianceoperator;
	ConstSubgridAdapter<T>	sga(*image, grid);
	return varianceoperator(sga);
}

/**
 * \brief retrieve the result image from the ImageMean object
 *
 * Makes the private image pointer accessible in the form of a
 * smart pointer. This method can only be called once, as image
 * is invalidate after the call.
 */
template<typename T>
ImagePtr	ImageMean<T>::getImagePtr() {
	ImagePtr	result(image);
	image = NULL;
	return result;
}

/**
 * \brief Perform dark computation for a subgrid
 */
template<typename T>
size_t	subdark(const ImageSequence&, ImageMean<T>& im,
	const Subgrid grid, unsigned int k = 3) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "processing subgrid %s",
		grid.toString().c_str());
	// we also need the mean of the image to decide which pixels are
	// too far off to consider them "sane" pixels
	T	mean = im.mean(grid);
	T	var = im.variance(grid);

	// now find out which pixels are bad, and mark them using NaNs.
	// we consider pixels bad if the deviate from the mean by more
	// than three standard deviations
	T	stddevk = k * sqrt(var);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found mean: %f, variance: %f, "
		"stddev%u = %f", mean, var, k, stddevk);
	size_t	badpixelcount = 0;
	SubgridAdapter<T>	sga(*im.image, grid);
	ImageSize	size = sga.getSize();
	for (unsigned int x = 0; x < size.width(); x++) {
		for (unsigned int y = 0; y < size.height(); y++) {
			T	v = sga.pixel(x, y);
			// skip NaNs
			if (v != v) {
				break;
			}
			if (fabs(v - mean) > stddevk) {
				sga.writablepixel(x, y)
					= std::numeric_limits<T>::quiet_NaN();
				badpixelcount++;
			}
		}
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %u bad pixels", badpixelcount);
	return badpixelcount;
}

/**
 * \brief Function to compute a dark image from a sequence of images
 *
 * This function first computes pixelwise mean and variance of the
 * image sequence. Then mean and variance over the image are computed.
 * This allows 
 * \param images	sequence of images to use to compute the 
 *			dark image
 */
template<typename T>
ImagePtr	dark_plain(const ImageSequence& images) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "plain dark processing");
	ImageMean<T>	im(images, true);
	subdark<T>(images, im, Subgrid());
	
	// that's it, we now have a dark image
	return im.getImagePtr();
}

template<typename T>
ImagePtr	dark(const ImageSequence& images, bool gridded = false) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "gridded: %s", (gridded) ? "YES" : "NO");
	if (!gridded) {
		return dark_plain<T>(images);
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "gridded dark processing");
	ImageMean<T>	im(images, true);
	// perform the dark computation for each individual subgrid
	size_t	badpixels = 0;
	ImageSize	step(2, 2);
	badpixels += subdark<T>(images, im, Subgrid(ImagePoint(0, 0), step));
	badpixels += subdark<T>(images, im, Subgrid(ImagePoint(1, 0), step));
	badpixels += subdark<T>(images, im, Subgrid(ImagePoint(0, 1), step));
	badpixels += subdark<T>(images, im, Subgrid(ImagePoint(1, 1), step));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "total bad pixels: %d", badpixels);
	return im.getImagePtr();
}

/**
 * \brief Dark image construction function for arbitrary image sequences
 */
ImagePtr DarkFrameFactory::operator()(const ImageSequence& images) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "processing %d images into dark frame",
		images.size());
	// make sure we have at least one image
	if (images.size() == 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot create dark from no images");
		throw std::runtime_error("no images in sequence");
	}

	// find out whether these are Bayer images, by looking at the first
	// image
	ImagePtr	firstimage = *images.begin();
	bool	gridded = firstimage->getMosaicType().isMosaic();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "first image is %sgridded",
		(gridded) ? "" : "not ");
	
	// based on the bit size of the first image, decide whether to work
	// with floats or with doubles
	unsigned int	floatlimit = std::numeric_limits<float>::digits;
	if (firstimage->bitsPerPixel() <= floatlimit) {
		return dark<float>(images, gridded);
	}
	return dark<double>(images, gridded);
}

/**
 * \brief Flat image construction function for arbitrary image sequences
 */
template<typename T>
ImagePtr	flat(const ImageSequence& images, const Image<T>& dark) {
	// we first compute the pixelwise mean, but we have to eliminate
	// possible cosmic ray artefacts, so we let the thing compute
	// the variance nevertheless
	debug(LOG_DEBUG, DEBUG_LOG, 0, "compute mean of images");
	ImageMean<T>	im(images, dark, true);

	// extract the image
	ImagePtr	result = im.getImagePtr();
	Image<T>	*image = dynamic_cast<Image<T> *>(&*result);

	// find the maximum value of the image
	Max<T, double>	maxfilter;
	T	maxvalue = maxfilter(*image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "maximum value: %f", maxvalue);

	// device the image by that value, so that the new maximum value
	// is 1
	for (unsigned int x = 0; x < image->size().width(); x++) {
		for (unsigned int y = 0; y < image->size().height(); y++) {
			image->pixel(x, y) /= maxvalue;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image normalized");

	return result;
}

ImagePtr	FlatFrameFactory::operator()(const ImageSequence& images,
			const ImagePtr& darkimage) const {
	Image<double>	*doubledark = dynamic_cast<Image<double>*>(&*darkimage);
	Image<float>	*floatdark = dynamic_cast<Image<float>*>(&*darkimage);
	if (doubledark) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "dark is Image<double>");
		CountNaNs<double, double>	countnans;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "dark has %f nans",
			countnans(*doubledark));
		return flat(images, *doubledark);
	}
	if (floatdark) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "dark is Image<float>");
		CountNaNs<float, double>	countnans;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "dark has %f nans",
			countnans(*floatdark));
		return flat(images, *floatdark);
	}
	throw std::runtime_error("unknown dark image type");
}

//////////////////////////////////////////////////////////////////////
// TypedCalibrator implementation (used for Calibrator)
//////////////////////////////////////////////////////////////////////
template<typename T>
class TypedCalibrator {
	const ConstImageAdapter<T>&	dark;
	const ConstImageAdapter<T>&	flat;
	T	nan;
public:
	TypedCalibrator(const ConstImageAdapter<T>& _dark,
		const ConstImageAdapter<T>& _flat);
	ImagePtr	operator()(const ImagePtr& image) const;
};

template<typename T>
TypedCalibrator<T>::TypedCalibrator(const ConstImageAdapter<T>& _dark,
	const ConstImageAdapter<T>& _flat) 
	: dark(_dark), flat(_flat) {
	nan = std::numeric_limits<T>::quiet_NaN();
}

template<typename T>
ImagePtr	TypedCalibrator<T>::operator()(const ImagePtr& image) const {
	ConstPixelValueAdapter<T>	im(image);
	Image<T>	*result = new Image<T>(image->size());
	for (unsigned int x = 0; x < image->size().width(); x++) {
		for (unsigned int y = 0; y < image->size().height(); y++) {
			T	darkvalue = dark.pixel(x, y);
			// if the pixel is bad give 
			if (darkvalue != darkvalue) {
				result->pixel(x, y) = nan;;
				continue;
			}
			T	v = im.pixel(x, y) - darkvalue;
			if (v < 0) {
				v = 0;
			}
			result->pixel(x, y) = v / flat.pixel(x, y);
		}
	}
	return ImagePtr(result);
}

//////////////////////////////////////////////////////////////////////
// Calibrator implementation
//////////////////////////////////////////////////////////////////////
Calibrator::Calibrator(const ImagePtr& _dark, const ImagePtr& _flat,
		const ImageRectangle _rectangle)
	: dark(_dark), flat(_flat), rectangle(_rectangle) {
	// We want dark and flat images to be of float or double type
	Image<float>	*fp = dynamic_cast<Image<float> *>(&*dark);
	Image<double>	*dp = dynamic_cast<Image<double> *>(&*dark);
	if ((fp == NULL) && (dp == NULL)) {
		std::string	msg("dark image must be of floating point type");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
}

ImagePtr	Calibrator::operator()(const ImagePtr& image) const {
	unsigned int	floatlimit = std::numeric_limits<float>::digits;
	// find the appropriate frame to use for the correction images
	ImageRectangle	frame;
	if (rectangle == ImageRectangle()) {
		frame = ImageRectangle(ImagePoint(), image->size());
	}

	// use pixel size to decide which type to use for the result image
	if (image->bitsPerPixel() <= floatlimit) {
		// create adapters for darks and flats with float values
		ConstPixelValueAdapter<float>	pvdark(dark);
		WindowAdapter<float>		wdark(pvdark, frame);
		ConstPixelValueAdapter<float>	pvflat(flat);
		WindowAdapter<float>		wflat(pvflat, frame);
		TypedCalibrator<float>	calibrator(wdark, wflat);
		return calibrator(image);
	}
	ConstPixelValueAdapter<double>	pvdark(dark);
	WindowAdapter<double>		wdark(pvdark, frame);
	ConstPixelValueAdapter<double>	pvflat(flat);
	WindowAdapter<double>		wflat(pvflat, frame);
	TypedCalibrator<double>	calibrator(wdark, wflat);
	return calibrator(image);
}

//////////////////////////////////////////////////////////////////////
// CalibrationFrameProcess implementation
//////////////////////////////////////////////////////////////////////

void	CalibrationFrameProcess::prepare() {
	// enable cooler, set temperature, if cooler available
	bool	usecooler = (ccd->hasCooler() && (_temperature > 0));
	if (usecooler) {
		CoolerPtr	cooler = ccd->getCooler();
		cooler->setTemperature(_temperature);
		cooler->setOn(true);

		// wait until temperature is close to set point
		while (fabs(cooler->getActualTemperature() - _temperature) > 1) {
			sleep(1);
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set temperature reached");
	}
}

void	CalibrationFrameProcess::cleanup() {
	bool	usecooler = (ccd->hasCooler() && (_temperature > 0));
	if (usecooler) {
		CoolerPtr	cooler = ccd->getCooler();
		cooler->setOn(false);
	}
}

//////////////////////////////////////////////////////////////////////
// DarkFrameProcess implementation
//////////////////////////////////////////////////////////////////////
ImagePtr	DarkFrameProcess::get() {
	prepare();

	// start exposure
	exposure.shutter = SHUTTER_CLOSED;
	ccd->startExposure(exposure);

	// get a sequence of images
	ImageSequence	images = ccd->getImageSequence(_nimages);

	// convert the images into a dark frame
	DarkFrameFactory	df;
	ImagePtr	dark = df(images);

	cleanup();

	// return the dark image
	return dark;
}

//////////////////////////////////////////////////////////////////////
// FlatFrameProcess implementation
//////////////////////////////////////////////////////////////////////
ImagePtr	FlatFrameProcess::get() {
	prepare();

	// start exposure
	exposure.shutter = SHUTTER_OPEN;
	ccd->startExposure(exposure);

	// get a sequence of images
	ImageSequence	images = ccd->getImageSequence(_nimages);

	// convert the images into a flat frame
	FlatFrameFactory	ff;
	ImagePtr	flat = ff(images, dark);

	// turn of cooler
	cleanup();

	// return the dark image
	return flat;
}



} // calibration
} // astro
