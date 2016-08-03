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

template<typename Pixel>
class Accumulator {
	Image<Pixel>&	image;
public:
	Accumulator(Image<Pixel>& _image) : image(_image) { }
	void	accumulate(const ConstImageAdapter<Pixel>& add);
};

template<typename Pixel>
void	Accumulator<Pixel>::accumulate(const ConstImageAdapter<Pixel>& add) {
	// check that dimensions match
	if (image.size() != add.getSize()) {
		throw std::runtime_error("image sizes in stack don't match");
	}

	// add new pixels
	for (int x = 0; x < image.size().width(); x++) {
		for (int y = 0; y < image.size().height(); y++) {
			image.pixel(x, y)
				= image.pixel(x, y) + add.pixel(x, y);
		}
	}
}

class MonochromeStacker {
	int	_patchsize;
public:
	MonochromeStacker(int patchsize) : _patchsize(patchsize) { }
	ImagePtr	operator()(ImageSequence images);
};

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
		TransformAnalyzer	ta(base, _patchsize, _patchsize);
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

#define	stacker_monochrome(image, pixel, images, patchsize)		\
	{								\
		Image<pixel>	*imagep					\
			= dynamic_cast<Image<pixel > *>(&*image);	\
		if (NULL != imagep) {					\
			MonochromeStacker	stacker(patchsize);	\
			return stacker(images);				\
		}							\
	}

template<typename Pixel>
class RGBStacker {
	int	_patchsize;
public:
	RGBStacker(int patchsize) : _patchsize(patchsize) { }
	ImagePtr	operator()(ImageSequence images);
};

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
	TransformAnalyzer	ta(base, _patchsize, _patchsize);

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

} // namespace stacking
} // namespace image
} // namespace astro
