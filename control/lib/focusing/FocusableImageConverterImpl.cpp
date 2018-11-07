/*
 * FocusableImageConverter.cpp -- Class to extract focusable float images
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <typeinfo>
#include <AstroFocus.h>
#include <AstroAdapter.h>
#include <AstroUtils.h>
#include <AstroIO.h>
#include "FocusableImageConverterImpl.h"

namespace astro {
namespace focusing {

/**
 * \brief Exception class thrown if the image cannot be handled
 */
class WrongImageType : public std::exception {
public:
	WrongImageType() { }
};

FocusableImageConverterImpl::FocusableImageConverterImpl() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "default converter uses full frame");
}

FocusableImageConverterImpl::FocusableImageConverterImpl(
		const ImageRectangle& rectangle)
	: _rectangle(rectangle) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "converter with rectangle %s",
		_rectangle.toString().c_str());
}

ImageRectangle	FocusableImageConverterImpl::rectangle_to_use(ImagePtr image) {
	if (ImageRectangle() != _rectangle) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using rectangle %s",
			_rectangle.toString().c_str());
		return _rectangle;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "using full frame %s",
		image->getFrame().toString().c_str());
	return ImageRectangle(image->size());
}

#define	raw_to_focusable(image, pixel)					\
{									\
	Image<pixel >	*img = dynamic_cast<Image<pixel > *>(&*image);	\
	if (NULL != img) {						\
		debug(LOG_DEBUG, DEBUG_LOG, 0, "raw %s",		\
			demangle(typeid(*img).name()).c_str());		\
		adapter::ConvertingAdapter<float, pixel > ca(*img);	\
		adapter::WindowAdapter<float>	wa(ca, r);		\
		return FocusableImage(new Image<float>(wa));		\
	}								\
}

FocusableImage	FocusableImageConverterImpl::get_raw(ImagePtr image) {
	ImageRectangle	r = rectangle_to_use(image);
	raw_to_focusable(image, unsigned char);
	raw_to_focusable(image, unsigned short);
	raw_to_focusable(image, unsigned int);
	raw_to_focusable(image, unsigned long);
	raw_to_focusable(image, float);
	raw_to_focusable(image, double);
	throw WrongImageType();
}

#define bayer_to_focusable(image, pixel)				\
{									\
	Image<pixel>	*img = dynamic_cast<Image<pixel>*>(&*image);	\
	if ((NULL != img) && (img->getMosaicType().isMosaic())) {	\
		debug(LOG_DEBUG, DEBUG_LOG, 0, "bayer %s",		\
			demangle(typeid(*img).name()).c_str());		\
		adapter::BayerGAdapter<pixel, float>	bga(img);	\
		adapter::WindowAdapter<float>	wa(bga, r);		\
		return FocusableImage(new Image<float>(wa));		\
	}								\
}

FocusableImage	FocusableImageConverterImpl::get_bayer(ImagePtr image) {
	ImageRectangle	r = rectangle_to_use(image);
	bayer_to_focusable(image, unsigned char);
	bayer_to_focusable(image, unsigned short);
	bayer_to_focusable(image, unsigned int);
	bayer_to_focusable(image, unsigned long);
	bayer_to_focusable(image, float);
	bayer_to_focusable(image, double);
	throw WrongImageType();
}

#define yuv_to_focusable(image, pixel)					\
{									\
	Image<YUV<pixel> >	*img					\
		= dynamic_cast<Image<YUV<pixel> >*>(&*image);		\
	if (NULL != img) {						\
		debug(LOG_DEBUG, DEBUG_LOG, 0, "yuv Image<YUV<%s>>",	\
			demangle(typeid(pixel).name()).c_str());	\
		adapter::YAdapter<pixel, float>	la(*img);		\
		adapter::WindowAdapter<float>	wa(la, r);		\
		return FocusableImage(new Image<float>(wa));		\
	}								\
}

FocusableImage	FocusableImageConverterImpl::get_yuv(ImagePtr image) {
	ImageRectangle	r = rectangle_to_use(image);
	yuv_to_focusable(image, unsigned char);
	yuv_to_focusable(image, unsigned short);
	yuv_to_focusable(image, unsigned int);
	yuv_to_focusable(image, unsigned long);
	throw WrongImageType();
}

#define rgb_to_focusable(image, pixel)					\
{									\
	Image<RGB<pixel> >	*img					\
		= dynamic_cast<Image<RGB<pixel> >*>(&*image);		\
	if (NULL != img) {						\
		debug(LOG_DEBUG, DEBUG_LOG, 0, "rgb %s",		\
			demangle(typeid(*img).name()).c_str());		\
		adapter::LuminanceAdapter<RGB<pixel>, float>	la(*img);	\
		adapter::WindowAdapter<float>	wa(la, r);		\
		return FocusableImage(new Image<float>(wa));		\
	}								\
}

FocusableImage	FocusableImageConverterImpl::get_rgb(ImagePtr image) {
	ImageRectangle	r = rectangle_to_use(image);
	rgb_to_focusable(image, unsigned char);
	rgb_to_focusable(image, unsigned short);
	rgb_to_focusable(image, unsigned int);
	rgb_to_focusable(image, unsigned long);
	rgb_to_focusable(image, float);
	rgb_to_focusable(image, double);
	throw WrongImageType();
}

FocusableImage	FocusableImageConverterImpl::operator()(const ImagePtr image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "apply %s->operator to %s image",
		_rectangle.toString().c_str(),
		image->getFrame().toString().c_str());
	FocusableImage	result;

	// handle Bayer images
	try {
		result = get_bayer(image);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found bayer image %s",
			result->getFrame().toString().c_str());
		goto meta;
	} catch (const WrongImageType& x) {
	}

	// handle raw images
	try {
		result = get_raw(image);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found raw image %s",
			result->getFrame().toString().c_str());
		goto meta;
	} catch (const WrongImageType& x) {
	}

	// handle YUV images
	try {
		result = get_yuv(image);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found yuv image %s",
			result->getFrame().toString().c_str());
		goto meta;
	} catch (const WrongImageType& x) {
	}

	// handle RGB images
	try {
		result = get_rgb(image);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found rgb image %s",
			result->getFrame().toString().c_str());
		goto meta;
	} catch (const WrongImageType& x) {
	}

	// unknown image type
	{
	std::string	imagetype = demangle(typeid(*image).name());
	std::string	msg = stringprintf("cannot extract focusable image "
		"from %s", imagetype.c_str());
	throw std::runtime_error(msg);
	}

meta:
	// copy the metadata except for the uuid
	result->metadata(image->metadata());
	if (result->hasMetadata(std::string("UUID"))) {
		std::string	v = result->getMetadata(std::string("UUID"));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "remove uuid %s", v.c_str());
		result->removeMetadata(std::string("UUID"));
		result->setMetadata(io::FITSKeywords::meta(std::string("UUID"), UUID()));
	}

	return result;
}

} // namespace focusing
} // namespace astro
