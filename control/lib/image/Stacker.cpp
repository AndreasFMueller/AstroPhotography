/*
 * Stacker.cpp -- stacker implementation
 *
 * (c) 2103 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroStacking.h>
#include <AstroAdapter.h>
#include <AstroDebug.h>
#include <AstroTransform.h>

using namespace astro::image;
using namespace astro::image::transform;
using namespace astro::adapter;

namespace astro {
namespace image {
namespace stacking {

/**
 * \brief Accumulator class to add images
 */
template<typename AccumulatorPixel, typename Pixel>
class Accumulator {
	ImagePtr	_image;		// used for resource management
	Image<AccumulatorPixel>	*_imageptr;
public:
	Accumulator(const ConstImageAdapter<Pixel> *baseimage) {
		if (NULL == baseimage) {
			return;
		}
		_imageptr = new Image<AccumulatorPixel>(*baseimage);
		_image = ImagePtr(_imageptr);
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
	// create a transform analyizer on the base image
	TypeConversionAdapter<Pixel>	baseimageadapter(*baseimage());

	// create an adapter to the target image to give it double values
	TypeConversionAdapter<Pixel>	targetimageadapter(image);

	// create an transform analyzer with respect to the base image
	TransformAnalyzer	transformanalyzer(baseimageadapter);
	transformanalyzer.patchsize(_patchsize);
	transformanalyzer.spacing(_patchsize);
	transformanalyzer.hanning(true);

	// get the transform
	Transform	transform = transformanalyzer(targetimageadapter);

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

#if 0
ImagePtr	MonochromeStacker::operator()(ImageSequence images) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start stacking of %d images",
		images.size());
	// get the base image
	ImagePtr	baseimage = *images.begin();
	ConstPixelValueAdapter<double>	base(baseimage);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "base image has size %s",
		baseimage->size().toString().c_str());

	// for each image in the sequence, find the transform relative to the
	// base image
	std::vector<Transform>	transforms;
	ImageSequence::const_iterator	imgp = images.begin();
	for (imgp++; imgp != images.end(); imgp++) {
		ImagePtr	imageptr = *imgp;
		ConstPixelValueAdapter<double>	img(imageptr);
		TransformAnalyzer	ta(base);
		ta.patchsize(_patchsize);
		ta.spacing(_patchsize);
		ta.hanning(true);
		Transform	t = ta(img);
		t = t.inverse();
		transforms.push_back(t);
	}

	// at this point, we have all the transforms
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d transforms",
		transforms.size());

	// prepare the result image, containing a copy of the base image
	Image<double>	*resultp = new Image<double>(base);
	ImagePtr	result(resultp);
	Accumulator<double>	accumulator(*resultp);

	// now compute a transformed double image for each image 
	// and add it to the base image
	imgp = images.begin();
	std::vector<Transform>::const_iterator	tp = transforms.begin();
	for (imgp++; imgp != images.end(); imgp++, tp++) {
		ImagePtr	imageptr = *imgp;
		ConstPixelValueAdapter<double>	img(imageptr);
		TransformAdapter<double>	ta(img, tp->inverse());
		accumulator.accumulate(ta);
	}

	return result;
}
#endif

#if 0
#define	stacker_monochrome(image, pixel, images, patchsize)		\
	{								\
		Image<pixel>	*imagep					\
			= dynamic_cast<Image<pixel > *>(&*image);	\
		if (NULL != imagep) {					\
			MonochromeStacker	stacker(patchsize);	\
			return stacker(images);				\
		}							\
	}
#endif

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

template<typename AccumulatorPixel, typename Pixel>
void	RGBStacker<AccumulatorPixel, Pixel>::add(
		const ConstImageAdapter<RGB<Pixel> >& image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stacking new image");
	// create a luminance adapter on the base image, because we only want
	// ot use the luminance when determining the transformation
	LuminanceAdapter<RGB<Pixel>, double>	luminancebase(*baseimage());

	// create an transform analyzer with respect to the base image
	TransformAnalyzer	transformanalyzer(luminancebase);
	transformanalyzer.patchsize(_patchsize);
	transformanalyzer.spacing(_patchsize);
	transformanalyzer.hanning(true);

	// find the transform to the new image
	LuminanceAdapter<RGB<Pixel>, double>	luminanceimage(image);
	Transform	transform = transformanalyzer(luminanceimage);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "transform: %s",
		transform.toString().c_str());

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

#if 0
template<typename Pixel>
ImagePtr	RGBStacker<Pixel>::operator()(ImageSequence images) {
	// get the base image
	ImagePtr	baseimage = *images.begin();
	Image<RGB<Pixel> >	*baseimagep
		= dynamic_cast<Image<RGB<Pixel> > *>(&*baseimage);
	if (NULL == baseimagep) {
		throw std::runtime_error("type inconsistency");
	}
	LuminanceAdapter<RGB<Pixel>, double>	base(*baseimagep);
	TransformAnalyzer	ta(base);
	ta.patchsize(_patchsize);
	ta.spacing(_patchsize);
	ta.hanning(true);

	// for each image in the sequence, find the transform relative to the
	// base image
	std::vector<Transform>	transforms;
	ImageSequence::const_iterator	imgp = images.begin();
	for (imgp++; imgp != images.end(); imgp++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "add image");
		ImagePtr	imageptr = *imgp;
		Image<RGB<Pixel > >	*imagep
			= dynamic_cast<Image<RGB<Pixel> > *>(&*imageptr);
		if (NULL == imagep) {
			throw std::runtime_error("image type inconsistency");
		}
		LuminanceAdapter<RGB<Pixel>, double>	img(*imagep);
		Transform	t = ta(img);
		transforms.push_back(t);
	}

	// at this point, we have all the transforms
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d transforms",
		transforms.size());

	// prepare the result image, containing a copy of the base image
	Image<RGB<double> >	*resultp = new Image<RGB<double> >(*baseimagep);
	ImagePtr	result(resultp);
	Accumulator<RGB<double> >	accumulator(*resultp);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "accumulator created");

	// now compute a transformed double image for each image 
	// and add it to the base image
	imgp = images.begin();
	std::vector<Transform>::const_iterator	tp = transforms.begin();
	for (imgp++; imgp != images.end(); imgp++, tp++) {
		ImagePtr	imageptr = *imgp;
		Image<RGB<Pixel> >	*imagep
			= dynamic_cast<Image<RGB<Pixel> > *>(&*imageptr);
		if (NULL == imagep) {
			throw std::runtime_error("image type inconsistenccy");
		}
		RGBAdapter<Pixel>	img(*imagep);
		TransformAdapter<RGB<double> >	ta(img, tp->inverse());
		accumulator.accumulate(ta);
	}

	return result;
}
#endif

#if 0
#define	stacker_rgb(image, pixel, images, patchsize)			\
	{								\
		Image<RGB<pixel > >	*imagep				\
			= dynamic_cast<Image<RGB<pixel > > *>(&*image);	\
		if (NULL != imagep) {					\
			RGBStacker<pixel>	stacker(patchsize);	\
			return stacker(images);				\
		}							\
	}


ImagePtr	Stacker::operator()(ImageSequence images) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get first image");
	// extract the baseimage, and construct a transform analyzer from
	// it the luminance channel associated with it
	ImagePtr	baseimage = *images.begin();

	// go through all possible types, and find an appropriately typed
	// stacker
	stacker_monochrome(baseimage, unsigned char, images, _patchsize);
	stacker_monochrome(baseimage, unsigned short, images, _patchsize);
	stacker_monochrome(baseimage, unsigned int, images, _patchsize);
	stacker_monochrome(baseimage, unsigned long, images, _patchsize);
	stacker_monochrome(baseimage, float, images, _patchsize);
	stacker_monochrome(baseimage, double, images, _patchsize);

	// color types
	stacker_rgb(baseimage, unsigned char, images, _patchsize);
	stacker_rgb(baseimage, unsigned short, images, _patchsize);
	stacker_rgb(baseimage, unsigned int, images, _patchsize);
	stacker_rgb(baseimage, unsigned long, images, _patchsize);
	stacker_rgb(baseimage, float, images, _patchsize);
	stacker_rgb(baseimage, double, images, _patchsize);

	throw std::runtime_error("cannot stack images of this type");
}
#endif

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
