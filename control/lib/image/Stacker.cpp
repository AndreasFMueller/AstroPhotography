/*
 * Stacker.cpp -- stacker implementation
 *
 * (c) 2103 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroStacking.h>
#include <AstroAdapter.h>
#include <AstroDebug.h>
#include <AstroTransform.h>
#include <AstroFilter.h>
#include <cmath>
#include "ReductionAdapter.h"

using namespace astro::image;
using namespace astro::image::transform;
using namespace astro::adapter;

namespace astro {
namespace image {
namespace stacking {

double	LOG(double x) {
	if (x <= 0) {
		return 0;
	}
	return log(x);
}

float	LOG(float x) {
	if (x <= 0) {
		return 0;
	}
	return logf(x);
}

/**
 * \brief Accumulator class to add images
 */
template<typename AccumulatorPixel, typename Pixel>
class Accumulator {
	ImagePtr	_image;		// used for resource management
	Image<AccumulatorPixel>	*_imageptr;
	int	_counter;
public:
	int	counter() const { return _counter; }
	Accumulator(const ConstImageAdapter<Pixel> *baseimage) {
		if (NULL == baseimage) {
			return;
		}
		_imageptr = new Image<AccumulatorPixel>(*baseimage);
		_image = ImagePtr(_imageptr);
		_counter = 0;
	}
	void	accumulate(const ConstImageAdapter<AccumulatorPixel>& add);
	ImagePtr	image() {
		return _image;
	}
};

template<typename AccumulatorPixel, typename Pixel>
void	Accumulator<AccumulatorPixel, Pixel>::accumulate(
		const ConstImageAdapter<AccumulatorPixel>& add) {
	// check that dimensions match
	if (_imageptr->size() != add.getSize()) {
		throw std::runtime_error("image sizes in stack don't match");
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "accumulating new image: %d",
		_counter++);

	// add new pixels
	int	w = _imageptr->size().width();
	int	h = _imageptr->size().height();
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			_imageptr->pixel(x, y)
				= _imageptr->pixel(x, y) + add.pixel(x, y);
		}
	}
}

Transform	Stacker::findtransform(const ConstImageAdapter<double>& base,
			const ConstImageAdapter<double>& image) const {
	// find the mean levels, this is used for the reduction later on
	double	mb = filter::Mean<double, double>().filter(base);
	double	mi = filter::Mean<double, double>().filter(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mb = %f, mi = %f", mb, mi);

	// we will need a transformation
	Transform	transform;

	// find out whether we should use triangles to find an initial transform
	if (usetriangles()) {
		// create an transform analyzer with respect to the base image
		TriangleAnalyzer	transformanalyzer(base, numberofstars(),
						searchradius());

		// find the transform between the base image and the new image
		transform = transformanalyzer.transform(image);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "initial transform: %s",
		transform.toString().c_str());

	// we now use this preliminary transform to improve using the Analyzer
	int	repeats = 3;
	while (repeats--) {
		TransformAdapter<double> transformedbase(base, transform);
		ReductionAdapter	reducedbase(transformedbase,
						mb, 2 * mb);
		Analyzer	analyzer(reducedbase);
		analyzer.patchsize(patchsize());
		analyzer.spacing(patchsize());
		analyzer.hanning(false);

		// now find the residuals to the target image
		ReductionAdapter	target(image, mi, 2 * mi);
		std::vector<Residual>	residuals = analyzer(target);

		// we only want to use residuals that are close
		int	counter = 0;
		auto ptr = residuals.begin();
		while (ptr != residuals.end()) {
			if (ptr->offset().abs() > residual()) {
				ptr = residuals.erase(ptr);
				counter++;
			} else {
				ptr++;
			}
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"excluded %d residuals too large", counter);

		// display the residuals that we still want to process
		if (debuglevel >= LOG_DEBUG) {
			int	i = 0;
			for (ptr = residuals.begin(); ptr != residuals.end();
				ptr++, i++) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"Residual[%d]: %s",
					i, std::string(*ptr).c_str());
			}
		}

		// create the improvement transform
		TransformFactory	tf(_rigid);
		Transform	deltatransform = tf(residuals);
		double	disc = deltatransform.discrepancy(image.getSize());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "delta transform: %s, disc = %f",
			deltatransform.toString().c_str(), disc);

		// the final transform is the composition of the previous 
		// transform with the deltatransform;
		transform = deltatransform.inverse() * transform;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "improved transform: %s",
		transform.toString().c_str());

		// check whether the difference is small enought so we can
		// give up
		if (disc < 2) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "accept transform, "
				"last discrepancy %f", disc);
			continue;
		}
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "final transform: %s, skew = %f",
		transform.toString().c_str(), transform.skew());
	return transform;
}

/**
 * \brief Stacker class for monochrome images
 */
template<typename AccumulatorPixel, typename Pixel>
class MonochromeStacker : public Stacker {
	const ConstImageAdapter<Pixel>	*baseimage() {
		return dynamic_cast<Image<Pixel>*>(&*_baseimage);
	}
	Accumulator<AccumulatorPixel, Pixel>	_accumulator;
public:
	MonochromeStacker(ImagePtr baseimage_ptr)
		: Stacker(baseimage_ptr), _accumulator(baseimage()) {
		if (NULL == baseimage()) {
			throw std::logic_error("base image type mismatch");
		}
	}
	void	add(const ConstImageAdapter<Pixel>& image);
	void	add(ImagePtr imageptr);
	ImagePtr	image() {
		return _accumulator.image();
	}
};

template<typename AccumulatorPixel, typename Pixel>
void	MonochromeStacker<AccumulatorPixel, Pixel>::add(
		const ConstImageAdapter<Pixel>& image) {
	// first handle the case where there is no transform
	if (notransform()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "accumulate with no transform");
		ConvertingAdapter<AccumulatorPixel, Pixel>	accumulatorimage(image);
		_accumulator.accumulate(accumulatorimage);
		return;
	}

	// create a transform analyizer on the base image
	TypeConversionAdapter<Pixel>	baseimageadapter(*baseimage());

	// create an adapter to the target image to give it double values
	TypeConversionAdapter<Pixel>	targetimageadapter(image);

	Transform	transform = findtransform(baseimageadapter,
					targetimageadapter);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add transform: %s",
		transform.toString().c_str());

	// create an adapter that converts the pixels of the original image
	// into pixels that are compatible with the accumulator
	ConvertingAdapter<AccumulatorPixel, Pixel>	accumulatorimage(image);

	// create an adapter that applies the transform to the image
	TransformAdapter<AccumulatorPixel>	transformadapter(
		accumulatorimage, transform.inverse());
	_accumulator.accumulate(transformadapter);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image added");
}

template<typename AccumulatorPixel, typename Pixel>
void	MonochromeStacker<AccumulatorPixel, Pixel>::add(ImagePtr imageptr) {
	Image<Pixel>	*imagep = dynamic_cast<Image<Pixel>*>(&*imageptr);
	if (NULL == imagep) {
		throw std::logic_error("new image has wrong type");
	}
	add(*imagep);
}

/**
 * \brief Stacker class for color images
 */
template<typename AccumulatorPixel, typename Pixel>
class RGBStacker : public Stacker {
	ImagePtr	_baseimageptr;
	const ConstImageAdapter<RGB<Pixel> >	*baseimage() {
		return dynamic_cast<Image<RGB<Pixel> >*>(&*_baseimageptr);
	}
	Accumulator<RGB<AccumulatorPixel>, RGB<Pixel> >	_accumulator;
public:
	RGBStacker(ImagePtr baseimageptr) : Stacker(baseimageptr),
		_baseimageptr(baseimageptr), _accumulator(baseimage()) {
		if (NULL == baseimage()) {
			std::string	cause = stringprintf("type mismatch");
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
			throw std::runtime_error(cause);
		}
	}

	void	add(const ConstImageAdapter<RGB<Pixel> >& image);

	void	add(ImagePtr imageptr);
	
	ImagePtr	image() {
		return _accumulator.image();
	}
};

/**
 * \brief Add an image to the stack
 *
 * This method computes a translation between two images
 */
template<typename AccumulatorPixel, typename Pixel>
void	RGBStacker<AccumulatorPixel, Pixel>::add(
		const ConstImageAdapter<RGB<Pixel> >& image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stacking new image");

	// first handle the case where there is no transform
	if (notransform()) {
		RGBAdapter<AccumulatorPixel, Pixel>	accumulatorimage(image);
		_accumulator.accumulate(accumulatorimage);
		return;
	}

	// create a luminance adapter on the base image, because we only want
	// ot use the luminance when determining the transformation
	LuminanceAdapter<RGB<Pixel>, double>	luminancebase(*baseimage());

	// find the transform to the new image
	LuminanceAdapter<RGB<Pixel>, double>	luminanceimage(image);
	Transform	transform = findtransform(luminancebase, luminanceimage);

	// create an adapter that converts the pixels of the original image
	// into pixels that are compatible with the accumulator
	RGBAdapter<AccumulatorPixel, Pixel>	accumulatorimage(image);

	// create an adapter that applies the transform to the image
	TransformAdapter<RGB<AccumulatorPixel> >	transformadapter(
		accumulatorimage, transform.inverse());
	_accumulator.accumulate(transformadapter);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image added");
}

template<typename AccumulatorPixel, typename Pixel>
void	RGBStacker<AccumulatorPixel, Pixel>::add(ImagePtr newimage) {
	// convert the image to a strongly typed image
	Image<RGB<Pixel> >	*imagep
		= dynamic_cast<Image<RGB<Pixel> >*>(&*newimage);
	if (NULL == imagep) {
		throw std::runtime_error("new image has wrong type");
	}

	// now add the image
	add(*imagep);
}

//////////////////////////////////////////////////////////////////////
// Implementation of the Stacker class
//////////////////////////////////////////////////////////////////////
#define	get_monochrome_stacker(baseimage, AccumulatorPixel, Pixel)	\
{									\
	Image<Pixel>	*imagep						\
		= dynamic_cast<Image<Pixel>*>(&*baseimage);		\
	if (NULL != imagep) {						\
		auto stacker = new MonochromeStacker<AccumulatorPixel,	\
				Pixel>(baseimage);			\
		return StackerPtr(stacker);				\
	}								\
}

#define	get_rgb_stacker(baseimage, AccumulatorPixel, Pixel)		\
{									\
	Image<RGB<Pixel> >	*imagep					\
		= dynamic_cast<Image<RGB<Pixel> >*>(&*baseimage);	\
	if (NULL != imagep) {						\
		auto stacker = new RGBStacker<AccumulatorPixel,		\
				Pixel>(baseimage);			\
		return StackerPtr(stacker);				\
	}								\
}

StackerPtr	Stacker::get(ImagePtr baseimage) {
	get_monochrome_stacker(baseimage, float, unsigned char);
	get_monochrome_stacker(baseimage, float, unsigned short);
	get_monochrome_stacker(baseimage, double, unsigned int);
	get_monochrome_stacker(baseimage, double, unsigned long);
	get_monochrome_stacker(baseimage, float, float);
	get_monochrome_stacker(baseimage, double, double);
	get_rgb_stacker(baseimage, float, unsigned char);
	get_rgb_stacker(baseimage, float, unsigned short);
	get_rgb_stacker(baseimage, double, unsigned int);
	get_rgb_stacker(baseimage, double, unsigned long);
	get_rgb_stacker(baseimage, float, float);
	get_rgb_stacker(baseimage, double, double);
	throw std::runtime_error("no stacker known for this image type");
}

} // namespace stacking
} // namespace image
} // namespace astro
