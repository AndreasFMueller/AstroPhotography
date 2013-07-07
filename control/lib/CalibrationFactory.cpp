/**
 * CalibrationFactory.cpp -- compute calibration frames
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCalibration.h>
#include <AstroFilter.h>
#include <limits>
#include <debug.h>
#include <stdexcept>
#include <vector>
#include <Format.h>

using namespace astro::image;
using namespace astro::image::filter;

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
		if ((*images.begin())->size != (*i)->size) {
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
 * \brief Adapter class for picture values of an ImagePtr
 *
 * This class allows access to the pixels of an image with primitive
 * pixel types, and performs an implicit type conversion to the type
 * we want to use in the calibration image computation.
 */
template<typename T>
class ConstPixelValue {
	const Image<unsigned char>	*byteimage;
	const Image<unsigned short>	*shortimage;
	const Image<unsigned int>	*intimage;
	const Image<unsigned long>	*longimage;
	const Image<float>	*floatimage;
	const Image<double>	*doubleimage;
public:
	ConstPixelValue(const ImagePtr& image);
	T	pixelvalue(unsigned int x, unsigned int y) const;
};

/**
 * \brief Constructor
 */
template<typename T>
ConstPixelValue<T>::ConstPixelValue(const ImagePtr& image) {
	byteimage = dynamic_cast<Image<unsigned char> *>(&*image);
	shortimage = dynamic_cast<Image<unsigned short> *>(&*image);
	intimage = dynamic_cast<Image<unsigned int> *>(&*image);
	longimage = dynamic_cast<Image<unsigned long> *>(&*image);
	floatimage = dynamic_cast<Image<float> *>(&*image);
	doubleimage = dynamic_cast<Image<double> *>(&*image);
	if ((NULL == byteimage) &&
	    (NULL == shortimage) &&
	    (NULL == intimage) &&
	    (NULL == longimage) &&
	    (NULL == floatimage) &&
	    (NULL == doubleimage)) {
		throw std::runtime_error("pixel type not primitive");
	}
}

/**
 * \brief Accessor to pixel value with implicit type conversion
 *
 * This method retrieves the pixel at point (x,y) and converts its
 * value to type T. If the conversion is not possible, a NaN is returned,
 * if available, or an exception thrown otherwise. Usually, calibration
 * images will be created with float or double types, so the exception
 * is not an issue.
 * \param x	x-coordinate of pixel
 * \param y	y-coordinate of pixel
 */
template<typename T>
T	ConstPixelValue<T>::pixelvalue(unsigned int x, unsigned int y) const {
	if (byteimage) {   return byteimage->pixelvalue<T>(x, y);   }
	if (shortimage) {  return shortimage->pixelvalue<T>(x, y);  }
	if (intimage) {    return intimage->pixelvalue<T>(x, y);    }
	if (longimage) {   return longimage->pixelvalue<T>(x, y);   }
	if (floatimage) {  return floatimage->pixelvalue<T>(x, y);  }
	if (doubleimage) { return doubleimage->pixelvalue<T>(x, y); }
	if (std::numeric_limits<T>::has_quiet_NaN) {
		return std::numeric_limits<T>::quiet_NaN();
	}
	throw std::runtime_error("NaN not available");
}

/**
 * \brief Access to pixel values
 *
 * This
 */
template<typename T>
class PixelValue {
	Image<unsigned char>	*byteimage;
	Image<unsigned short>	*shortimage;
	Image<unsigned int>	*intimage;
	Image<unsigned long>	*longimage;
	Image<float>	*floatimage;
	Image<double>	*doubleimage;
public:
	PixelValue(ImagePtr& image);
	T	pixelvalue(unsigned int x, unsigned int y);
};

template<typename T>
PixelValue<T>::PixelValue(ImagePtr& image) {
	byteimage = dynamic_cast<Image<unsigned char> *>(&*image);
	shortimage = dynamic_cast<Image<unsigned short> *>(&*image);
	intimage = dynamic_cast<Image<unsigned int> *>(&*image);
	longimage = dynamic_cast<Image<unsigned long> *>(&*image);
	floatimage = dynamic_cast<Image<float> *>(&*image);
	doubleimage = dynamic_cast<Image<double> *>(&*image);
	if ((NULL == byteimage) &&
	    (NULL == shortimage) &&
	    (NULL == intimage) &&
	    (NULL == longimage) &&
	    (NULL == floatimage) &&
	    (NULL == doubleimage)) {
		throw std::runtime_error("pixel type not primitive");
	}
}

template<typename T>
T	PixelValue<T>::pixelvalue(unsigned int x, unsigned int y) {
	if (byteimage) {   return byteimage->pixelvalue<T>(x, y);   }
	if (shortimage) {  return shortimage->pixelvalue<T>(x, y);  }
	if (intimage) {    return intimage->pixelvalue<T>(x, y);    }
	if (longimage) {   return longimage->pixelvalue<T>(x, y);   }
	if (floatimage) {  return floatimage->pixelvalue<T>(x, y);  }
	if (doubleimage) { return doubleimage->pixelvalue<T>(x, y); }
	if (std::numeric_limits<T>::has_quiet_NaN) {
		return std::numeric_limits<T>::quiet_NaN();
	}
	throw std::runtime_error("NaN not available");
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
public:
	typedef ConstPixelValue<T>	PV;
	typedef std::vector<PV>	PVSequence;

private:
	PVSequence	pvs;
	/**
	 * \brief Prepare internal data
	 *
 	 * This method is called to set up the PixelValue vectors
	 */
	void	setup_pv(const ImageSequence& images) {
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
	/**
	 * \brief Prepare internal data for dark image compuation
	 */
	void	setup_images(const ImageSequence& images) {
		// create an image of appropriate size
		size = (*images.begin())->size;
		image = new Image<T>(size);
		if (enableVariance) {
			// prepare the variance image
			var = new Image<T>(size);
		} else {
			var = NULL;
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
	void	compute(unsigned int x, unsigned int y, T darkvalue) {
		// if the dark value is invalid, then the computed value
		// is also invalid
		if (darkvalue != darkvalue) {
			image->pixel(x, y) = darkvalue;
			var->pixel(x, y) = darkvalue;
			return;
		}

		// perform mean (and possibly variance) computation in the
		// case where 
		T	m;
		T	X = 0, X2 = 0;
		typename std::vector<PV>::const_iterator j;
		int	counter = 0;
		for (j = pvs.begin(); j != pvs.end(); j++) {
			T	v = j->pixelvalue(x, y);
			// skip this value if it is a NaN
			if (v != v)
				continue;
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
		T	stddev3 = 3 * sqrt(EX2 - EX * EX);
		if (stddev3 < 1) {
			stddev3 = std::numeric_limits<T>::infinity();
		}
if (stddev3 < 1) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "EX = %f, EX2 = %f", EX, EX2);
}
		for (j = pvs.begin(); j != pvs.end(); j++) {
			T	v = j->pixelvalue(x, y);
			// skip NaNs
			if (v != v)
				continue;
			// skip values that are too far off
			if (fabs(v - EX) > stddev3) {
debug(LOG_DEBUG, DEBUG_LOG, 0, "(%d,%d): skipping %f, EX = %f, stddev3 = %f", x, y, v, EX, stddev3);
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

public:
	/**
	 * \brief Constructor for ImageMean object
	 *
	 * The constructor remembers all images, sets up PixelValue objects
	 * for them, and computes mean and variance for each point
	 * \param images	a sequence of images
	 * \param _enableVariance	whether or not the variance should be
	 *				computed
 	 */
	ImageMean(const ImageSequence& images, bool _enableVariance = false)
		: enableVariance(_enableVariance) {
		// compute the PixelValue objects
		setup_pv(images);

		// allocate the images
		setup_images(images);

		// now compute mean and variance for every pixel
		for (unsigned int x = 0; x < size.width; x++) {
			for (unsigned int y = 0; y < size.height; y++) {
				compute(x, y, 0);
			}
		}
	}

	/**
	 * \brief Construtor for ImageMean object with dark value correction
	 * 
	 * Constructs an ImageMean object, but ignores pixels where the
	 * dark image has NaN values. This allows to first construct a
	 * map of dark pixels, which should be ignored, and then perform
	 * the computation of the dark images ignoring the bad pixels.
 	 */
	ImageMean(const ImageSequence& images, const Image<T>& dark,
		bool _enableVariance = false)
		: enableVariance(_enableVariance) {
		// compute the PixelValue objects
		setup_pv(images);

		// allocate the images
		setup_images(images);

		// now compute mean and variance for every pixel
		for (unsigned int x = 0; x < size.width; x++) {
			for (unsigned int y = 0; y < size.height; y++) {
				T	darkvalue = dark.pixel(x, y);
				compute(x, y, darkvalue);
			}
		}
	}

	~ImageMean() {
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
	T	mean() const {
		Mean<T, T>	meanoperator;
		return meanoperator(*image);
	}

	/**
	 * \brief compute variance of the result image
	 */
	T	variance() const {
		Variance<T, T>	varianceoperator;
		return varianceoperator(*image);
	}

	/**
	 * \brief retrieve the result image from the ImageMean object
	 *
 	 * Makes the private image pointer accessible in the form of a
	 * smart pointer. This method can only be called once, as image
	 * is invalidate after the call.
	 */
	ImagePtr	getImagePtr() {
		ImagePtr	result(image);
		image = NULL;
		return result;
	}
};

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
ImagePtr	dark(const ImageSequence& images) {
	ImageMean<T>	im(images, true);
	
	// we also need the mean of the image to decide which pixels are
	// too far off to consider them "sane" pixels
	T	mean = im.mean();
	T	var = im.variance();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found mean: %f, variance: %f",
		mean, var);

	// now find out which pixels are bad, and mark them using NaNs.
	// we consider pixels bad if the deviate from the mean by more
	// than three standard deviations
	T	stddev3 = 3 * sqrt(var);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stddev3 = %f", stddev3);
	unsigned int	badpixelcount = 0;
	for (unsigned int x = 0; x < im.size.width; x++) {
		for (unsigned int y = 0; y < im.size.height; y++) {
			if (fabs(im.image->pixel(x, y) - mean) > stddev3) {
				im.image->pixel(x, y)
					= std::numeric_limits<T>::quiet_NaN();
				badpixelcount++;
			}
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %u bad pixels", badpixelcount);

	// that's it, we now have a dark image
	return im.getImagePtr();
}

/**
 * \brief Dark image construction function for arbitrary image sequences
 */
ImagePtr DarkFrameFactory::operator()(const ImageSequence& images) const {
	if ((*images.begin())->bitsPerPixel() <= std::numeric_limits<float>::digits) {
		return dark<float>(images);
	}
	return dark<double>(images);
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
	for (unsigned int x = 0; x < image->size.width; x++) {
		for (unsigned int y = 0; y < image->size.height; y++) {
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
	ConstPixelValue<T>	dark;
	ConstPixelValue<T>	flat;
	T	nan;
public:
	TypedCalibrator(const ImagePtr& _dark, const ImagePtr& _flat);
	ImagePtr	operator()(const ImagePtr& image) const;
};

template<typename T>
TypedCalibrator<T>::TypedCalibrator(const ImagePtr& _dark,
	const ImagePtr& _flat) : dark(_dark), flat(_flat) {
	nan = std::numeric_limits<T>::quiet_NaN();
}

template<typename T>
ImagePtr	TypedCalibrator<T>::operator()(const ImagePtr& image) const {
	ConstPixelValue<T>	im(image);
	Image<T>	*result = new Image<T>(image->size);
	for (unsigned int x = 0; x < image->size.width; x++) {
		for (unsigned int y = 0; y < image->size.height; y++) {
			T	darkvalue = dark.pixelvalue(x, y);
			// if the pixel is bad give 
			if (darkvalue != darkvalue) {
				result->pixel(x, y) = nan;;
				continue;
			}
			result->pixel(x, y) = (im.pixelvalue(x, y) - darkvalue)
				/ flat.pixelvalue(x, y);
		}
	}
	return ImagePtr(result);
}

//////////////////////////////////////////////////////////////////////
// Clamp images to a given range
//////////////////////////////////////////////////////////////////////
Clamper::Clamper(double _minvalue, double _maxvalue)
	: minvalue(_minvalue), maxvalue(_maxvalue) {
}

template<typename P>
void	do_clamp(Image<P>& image, double minvalue, double maxvalue) {
	for (size_t offset = 0; offset < image.size.pixels; offset++) {
		P	value = image.pixels[offset];
		// skip indefined pixels
		if (value != value) {
			continue;
		}
		if (value < minvalue) {
			value = minvalue;
		}
		if (value > maxvalue) {
			value = maxvalue;
		}
		image.pixels[offset] = value;
	}
}

#define	do_clamp_typed(P)						\
{									\
	Image<P>	*timage = dynamic_cast<Image<P> *>(&*image);	\
	if (NULL != timage) {						\
		do_clamp(*timage, minvalue, maxvalue);			\
		return;							\
	}								\
}

void	Clamper::operator()(ImagePtr& image) const {
	do_clamp_typed(unsigned char);
	do_clamp_typed(unsigned short);
	do_clamp_typed(unsigned int);
	do_clamp_typed(unsigned long);
	do_clamp_typed(float);
	do_clamp_typed(double);
}

//////////////////////////////////////////////////////////////////////
// Type dark correctors
//
// Dark correction can be applied to any type of image, with varying
// primitive pixel types. These templates perform dark correction
// based on the various possible pixel types
//////////////////////////////////////////////////////////////////////
template<typename ImagePixelType, typename DarkPixelType>
void	dark_correct(Image<ImagePixelType>& image,
		const Image<DarkPixelType>& dark) {
	// first check that image sizes match
	if (image.size != dark.size) {
		std::string	msg = stringprintf("size: image %s != dark %s",
			image.size.toString().c_str(),
			dark.size.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// correct all pixels
	for (size_t offset = 0; offset < image.size.pixels; offset++) {
		ImagePixelType	ip = image.pixels[offset];
		// skip NaN pixels
		if (ip != ip) {
			continue;
		}
		DarkPixelType	dp = dark.pixels[offset];
		// turn off (make nan) pixels that are marked nan in the dark
		if (dp != dp) {
			ip = 0;
		} else {
			if (ip > dp) {
				ip = ip - dp;
			} else {
				ip = 0;
			}
		}
		image.pixels[offset] = ip;
	}
}

#define	dark_correct_for(T)						\
{									\
	Image<T>	*timage	= dynamic_cast<Image<T> *>(&*image);	\
	if (NULL != timage) {						\
		dark_correct(*timage, dark);				\
		return;							\
	}								\
}

template<typename DarkPixelType>
void	dark_correct_typed(ImagePtr& image,
		const Image<DarkPixelType>& dark) {
	dark_correct_for(unsigned char);
	dark_correct_for(unsigned short);
	dark_correct_for(unsigned int);
	dark_correct_for(unsigned long);
	dark_correct_for(double);
	dark_correct_for(float);
}

//////////////////////////////////////////////////////////////////////
// DarkCorrector implementation
//////////////////////////////////////////////////////////////////////
DarkCorrector::DarkCorrector(const ImagePtr& _dark) : dark(_dark) {
	// We want dark images to be of float or double type
	Image<float>	*fp = dynamic_cast<Image<float> *>(&*dark);
	Image<double>	*dp = dynamic_cast<Image<double> *>(&*dark);
	if ((NULL != fp) || (NULL != dp)) {
		return;
	}
	std::string	msg("dark image must be of floating point type");
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief perform dark correction
 *
 * Subtract the dark image from the argument image. This is done in place,
 * as most quite, the uncorrected image is no longer needed. If a new image
 * is required, first create the new image, then apply the dark corrector in
 * place.
 * \param image     image to dark correct
 */
void	DarkCorrector::operator()(ImagePtr& image) const {
	Image<float>	*fp = dynamic_cast<Image<float> *>(&*dark);
	Image<double>	*dp = dynamic_cast<Image<double> *>(&*dark);
	if (NULL != fp) {
		dark_correct_typed<float>(image, *fp);
		return;
	}
	if (NULL != dp) {
		dark_correct_typed<double>(image, *dp);
		return;
	}
	std::string	msg("dark image must be of floating point type");
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

//////////////////////////////////////////////////////////////////////
// Calibrator implementation
//////////////////////////////////////////////////////////////////////
Calibrator::Calibrator(const ImagePtr& _dark, const ImagePtr& _flat)
	: dark(_dark), flat(_flat) {
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
	if (image->bitsPerPixel() <= std::numeric_limits<float>::digits) {
		TypedCalibrator<float>	calibrator(dark, flat);
		return calibrator(image);
	}
	TypedCalibrator<double>	calibrator(dark, flat);
	return calibrator(image);
}

//////////////////////////////////////////////////////////////////////
// TypedInterpolator implementation
//////////////////////////////////////////////////////////////////////
template<typename T>
class TypedInterpolator {
	const Image<T>&	dark;
protected:
	virtual void	interpolatePixel(unsigned int x, unsigned int y,
				ImagePtr& image);
	PixelValue<T>	*pv;
	T	nan;
public:
	TypedInterpolator(const Image<T>& _dark);
	void	interpolate(ImagePtr& image);
};

template<typename T>
void	TypedInterpolator<T>::interpolatePixel(unsigned int x, unsigned int y,
		ImagePtr& image) {
	// XXX interpolation missing
}

template<typename T>
TypedInterpolator<T>::TypedInterpolator(const Image<T>& _dark) : dark(_dark) {
	nan = std::numeric_limits<T>::quiet_NaN();
}

template<typename T>
void	TypedInterpolator<T>::interpolate(ImagePtr& image) {
	// make sure the image sizes match
	if (image->size != dark.size) {
		throw std::range_error("image sizes don't match");
	}
	pv = new PixelValue<T>(image);
	for (unsigned int x = 0; x < dark.size.width; x++) {
		for (unsigned int y = 0; y < dark.size.height; y++) {
			if (dark.pixel(x, y) != dark.pixel(x, y)) {
				interpolatePixel(x, y, image);
			}
		}
	}
	delete pv;
}

//////////////////////////////////////////////////////////////////////
// Interpolator implementation
//////////////////////////////////////////////////////////////////////
Interpolator::Interpolator(const ImagePtr& _dark) : dark(_dark) {
	floatdark = dynamic_cast<Image<float> *>(&*dark);
	doubledark = dynamic_cast<Image<double> *>(&*dark);
	if ((NULL == floatdark) && (NULL == doubledark)) {
		throw std::runtime_error("only float or double images are suitable as darks");
	}
}

void	Interpolator::interpolate(ImagePtr& image) {
	if (floatdark) {
		TypedInterpolator<float>	tint(*floatdark);
		tint.interpolate(image);
		return;
	}
	if (doubledark) {
		TypedInterpolator<double>	tint(*doubledark);
		tint.interpolate(image);
		return;
	}
}

ImagePtr	Interpolator::operator()(const ImagePtr& image) {
	ImagePtr	imagecopy;
	interpolate(imagecopy);
	return imagecopy;
}

} // calibration
} // astro
