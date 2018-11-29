/**
 * ImageMean.h
 *
 * (c) 2018 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ImageMean_h
#define _ImageMean_h

#include <includes.h>
#include <AstroCalibration.h>
#include <AstroFilter.h>
#include <AstroFilterfunc.h>
#include <PixelValue.h>
#include <limits>
#include <AstroDebug.h>
#include <stdexcept>
#include <vector>
#include <AstroFormat.h>

using namespace astro::image::filter;
using namespace astro::adapter;

namespace astro {
namespace calibration {

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
	bool	consistent(const ImageSequence& images);

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
private:
	ImagePtr	imageptr;
public:

	/**
	 * \brief Variance per pixel
	 *
	 * This image contains the variance of pixel values at the same position
 	 */
	Image<T>	*var;
private:
	ImagePtr	varptr;

private:
	void	setup_images(const ImageSequence& images);
	void	compute(int x, int y, T darkvalue);
	void	statistics();

public:
	ImageMean(const ImageSequence& images, bool _enableVariance = false);
	ImageMean(const ImageSequence& images, const Image<T>& dark,
		bool _enableVariance = false);

	T	mean(const Subgrid grid = Subgrid()) const;
	T	mean(const ImageRectangle& rectangle,
			const Subgrid grid = Subgrid()) const;
	T	variance(const Subgrid grid = Subgrid()) const;
	T	variance(const ImageRectangle& rectangle,
			const Subgrid grid = Subgrid()) const;
	ImagePtr	getImagePtr();
};

/**
 * \brief Prepare internal data for dark image compuation
 */
template<typename T>
void	ImageMean<T>::setup_images(const ImageSequence& images) {
	// create an image of appropriate size
	ImagePtr	firstimage = *images.begin();
	size = firstimage->size();

	// result image
	image = new Image<T>(size);
	imageptr = ImagePtr(image);

	// variance (if needed
	if (enableVariance) {
		// prepare the variance image
		var = new Image<T>(size);
		varptr = ImagePtr(var);
	} else {
		var = NULL;
		varptr = ImagePtr();
	}

	// mosaic info
	imageptr->setMosaicType(firstimage->getMosaicType());

	// do we have filter information?
	if (firstimage->hasMetadata("FILTER")) {
		imageptr->setMetadata(firstimage->getMetadata("FILTER"));
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
 * \brief Check that the image sequence is consistent 
 *
 * Only a if all the images are of the same size we can actually compute
 * a calibration image.
 * \param images
 */
template<typename T>
bool	ImageMean<T>::consistent(const ImageSequence& images) {
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
 * \brief Perform dark image computation per pixel
 *
 * Computes mean and variance (if enabled) of the pixels
 * at point (x,y) from all images in the image sequence.
 * The PixelValue objects are used for this purpose.
 * \param x	x-coordinate of pixel
 * \param y	y-coordinate of pixel
 */
template<typename T>
void	ImageMean<T>::compute(int x, int y, T darkvalue) {
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
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"bad pixel values at (%d, %d): %d", x, y, counter);
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
	for (int x = 0; x < size.width(); x++) {
		for (int y = 0; y < size.height(); y++) {
			compute(x, y, 0);
		}
	}

	// number of standard deviations for bad pixels
	k = 3;

	// compute statistics
	statistics();
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
	for (int x = 0; x < size.width(); x++) {
		for (int y = 0; y < size.height(); y++) {
			T	darkvalue = dark.pixel(x, y);
			compute(x, y, darkvalue);
		}
	}

	// number of standard deviations for bad pixels
	k = 3;

	// compute statistics
	statistics();
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

template<typename T>
T	ImageMean<T>::mean(const ImageRectangle& rectangle,
		const Subgrid grid) const {
	Mean<T, T>	meanoperator;
	WindowAdapter<T>	wa(*image, rectangle);
	ConstSubgridAdapter<T>	sga(wa, grid);
	return meanoperator(sga);
}

/**
 * \brief compute variance of the result image
 *
 * \param grid	the subgrid to evaluate
 */
template<typename T>
T	ImageMean<T>::variance(const Subgrid grid) const {
	Variance<T, T>	varianceoperator;
	ConstSubgridAdapter<T>	sga(*image, grid);
	return varianceoperator(sga);
}

template<typename T>
T	ImageMean<T>::variance(const ImageRectangle& rectangle,
		const Subgrid grid) const {
	Variance<T, T>	varianceoperator;
	WindowAdapter<T>	wa(*image, rectangle);
	ConstSubgridAdapter<T>	sga(wa, grid);
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
	return imageptr;
}

/**
 * \brief Compute statistics values
 */
template<typename T>
void	ImageMean<T>::statistics() {
	if (imageptr->getMosaicType() != MosaicType()) {
		// get statistics from an RGB image
		RGB<double>	min = min_color(imageptr);
		imageptr->setMetadata(io::FITSKeywords::meta("MIN-R", min.R));
		imageptr->setMetadata(io::FITSKeywords::meta("MIN-G", min.G));
		imageptr->setMetadata(io::FITSKeywords::meta("MIN-B", min.B));

		RGB<double>	max = max_color(imageptr);
		imageptr->setMetadata(io::FITSKeywords::meta("MAX-R", max.R));
		imageptr->setMetadata(io::FITSKeywords::meta("MAX-G", max.G));
		imageptr->setMetadata(io::FITSKeywords::meta("MAX-B", max.B));

		RGB<double>	mn = mean_color(imageptr);
		imageptr->setMetadata(io::FITSKeywords::meta("MEAN-R", mn.R));
		imageptr->setMetadata(io::FITSKeywords::meta("MEAN-G", mn.G));
		imageptr->setMetadata(io::FITSKeywords::meta("MEAN-B", mn.B));

		return;
	}

	// compute the max/min/mean values
	double	minval = filter::min(imageptr);
	double	maxval = filter::max(imageptr);
	double	mnval = filter::mean(imageptr);

	// do we have filter information?
	std::string	filtername;
	if (imageptr->hasMetadata("FILTER")) {
		filtername = trim(imageptr->getMetadata("FILTER").getValue());
	}
	if (filtername == "R") {
		imageptr->setMetadata(io::FITSKeywords::meta("MIN-R", minval));
		imageptr->setMetadata(io::FITSKeywords::meta("MAX-R", maxval));
		imageptr->setMetadata(io::FITSKeywords::meta("MEAN-R", mnval));
	} else if (filtername == "G") {
		imageptr->setMetadata(io::FITSKeywords::meta("MIN-G", minval));
		imageptr->setMetadata(io::FITSKeywords::meta("MAX-G", maxval));
		imageptr->setMetadata(io::FITSKeywords::meta("MEAN-G", mnval));
	} else if (filtername == "B") {
		imageptr->setMetadata(io::FITSKeywords::meta("MIN-B", minval));
		imageptr->setMetadata(io::FITSKeywords::meta("MAX-B", maxval));
		imageptr->setMetadata(io::FITSKeywords::meta("MEAN-B", mnval));
	} else {
		imageptr->setMetadata(io::FITSKeywords::meta("MIN", minval));
		imageptr->setMetadata(io::FITSKeywords::meta("MAX", maxval));
		imageptr->setMetadata(io::FITSKeywords::meta("MEAN", mnval));
	}
}

} // calibration
} // astro

#endif /* _ImageMean_h */
