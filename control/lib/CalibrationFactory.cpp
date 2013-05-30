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

ImagePtr	CalibrationFrameFactory::operator()(const ImageSequence& images) const {
	return ImagePtr();
}

/**
 * \brief Adapter class for picture values of an ImagePtr
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
 * This class is needed by several methods that compute means, variations
 * and medians to decide whether or not to consider an image pixel as valid
 */
template<typename T>
class ImageMean {
	bool	enableVariance;
public:
	typedef ConstPixelValue<T>	PV;
	typedef std::vector<PV>	PVSequence;

private:
	PVSequence	pvs;
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
	Image<T>	*image;
	Image<T>	*var;

private:
	void	setup_images(const ImageSequence& images) {
		// create an image of appropriate size
		size = (*images.begin())->size;
		image = new Image<T>(size);
		if (enableVariance) {
			var = new Image<T>(size);
		} else {
			var = NULL;
		}
	}

	void	compute(unsigned int x, unsigned int y, T darkvalue) {
		// if the dark value is invalid, then the computed value
		// is also invalid
		if (std::numeric_limits<T>::quiet_NaN() == darkvalue) {
			image->pixel(x, y) = darkvalue;
			var->pixel(x, y) = darkvalue;
			return;
		}

		// perform mean (and possibly variance) computation in the
		// case where 
		T	X = 0, X2 = 0;
		typename std::vector<PV>::const_iterator j;
		for (j = pvs.begin(); j != pvs.end(); j++) {
			T	v = j->pixelvalue(x, y);
			X += v;
			if (enableVariance) {
				X2 += v * v;
			}
		}
		T	EX = X / pvs.size();
		image->pixel(x, y) = EX;
		if (enableVariance) {
			T	EX2 = X2 / pvs.size();
			var->pixel(x, y) = EX2 - EX * EX;
		}
	}

public:
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
	
	T	mean() const {
		Mean<T, T>	meanoperator;
		return meanoperator(*image);
	}

	T	variance() const {
		Mean<T, T>	meanoperator;
		return meanoperator(*var);
	}

	ImagePtr	getImagePtr() {
		ImagePtr	result(image);
		image = NULL;
		return result;
	}
};

template<typename T>
ImagePtr	dark(const ImageSequence& images) {
	ImageMean<T>	im(images, true);
	
	// we also need the mean of the image to decide which pixels are
	// too far off to consider them "sane" pixels
	T	mvar = im.mean();
	T	m = im.variance();

	// now find out which pixels are bad, and mark them using NaNs.
	// we consider pixels bad if the deviate from the mean by more
	// than three standard deviations
	T	stddev3 = 3 * sqrt(mvar);
	unsigned int	badpixelcount = 0;
	for (unsigned int x = 0; x < im.size.width; x++) {
		for (unsigned int y = 0; y < im.size.height; y++) {
			if (fabs(im.image->pixel(x, y) - m) > stddev3) {
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

ImagePtr DarkFrameFactory::operator()(const ImageSequence& images) const {
	if ((*images.begin())->bitsPerPixel() <= std::numeric_limits<float>::digits) {
		return dark<float>(images);
	}
	return dark<double>(images);
}

template<typename T>
ImagePtr	flat(const ImageSequence& images, const Image<T>& dark) {
	ImageMean<T>	im(images, dark, false); // variance not needed
	ImagePtr	result = im.getImagePtr();
	Image<T>	*image = dynamic_cast<Image<T>*>(&*result);
	if (NULL == image) {
		throw std::runtime_error("dark image type mismatch");
	}

	// find the maximum value of the image
	Max<T>	maxfilter;
	T	maxvalue = maxfilter(*image);

	// device the image by that value, so that the new maximum value
	// is 1
	for (unsigned int x = 0; x < image->size.width; x++) {
		for (unsigned int y = 0; y < image->size.height; y++) {
			image->pixel(x, y) /= maxvalue;
		}
	}

	return im.getImagePtr();
}

ImagePtr	FlatFrameFactory::operator()(const ImageSequence& images,
			const ImagePtr& darkimage) const {
	Image<double>	*doubledark = dynamic_cast<Image<double>*>(&*darkimage);
	Image<float>	*floatdark = dynamic_cast<Image<float>*>(&*darkimage);
	if (doubledark) {
		return flat(images, *doubledark);
	}
	if (floatdark) {
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
			if (darkvalue == nan) {
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
// Calibrator implementation
//////////////////////////////////////////////////////////////////////
Calibrator::Calibrator(const ImagePtr& _dark, const ImagePtr& _flat)
	: dark(_dark), flat(_flat) {
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
			if (dark.pixel(x, y) == nan) {
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
