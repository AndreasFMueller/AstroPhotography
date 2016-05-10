/*
 * FocusableImageConverter.cpp -- Class to extract focusable float images
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <typeinfo>
#include <AstroFocus.h>
#include <AstroAdapter.h>
#include <AstroUtils.h>

namespace astro {
namespace focusing {

/**
 * \brief Exception class thrown if the image cannot be handled
 */
class WrongImageType : public std::exception {
public:
	WrongImageType() { }
};

//////////////////////////////////////////////////////////////////////
// Implementation class for the FocusableImage extraction
//////////////////////////////////////////////////////////////////////

/**
 * \brief Class to hide implementation details 
 *
 * This class actually implements the image conversion
 */
class FocusableImageConverterImpl : public FocusableImageConverter {
	FocusableImage	get_raw(ImagePtr image);
	FocusableImage	get_bayer(ImagePtr image);
	FocusableImage	get_yuv(ImagePtr image);
	FocusableImage	get_rgb(ImagePtr image);
public:
	FocusableImageConverterImpl() { }
	FocusableImage	operator()(ImagePtr image);
};

#define	raw_to_focusable(image, pixel)					\
{									\
	Image<pixel >	*img = dynamic_cast<Image<pixel > *>(&*image);	\
	if (NULL != img) {						\
		debug(LOG_DEBUG, DEBUG_LOG, 0, "raw %s",		\
			demangle(typeid(*img).name()).c_str());		\
		adapter::ConvertingAdapter<float, pixel > ca(*img);	\
		return FocusableImage(new Image<float>(ca));		\
	}								\
}

FocusableImage	FocusableImageConverterImpl::get_raw(ImagePtr image) {
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
		return FocusableImage(new Image<float>(bga));		\
	}								\
}

FocusableImage	FocusableImageConverterImpl::get_bayer(ImagePtr image) {
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
		return FocusableImage(new Image<float>(la));		\
	}								\
}

FocusableImage	FocusableImageConverterImpl::get_yuv(ImagePtr image) {
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
		return FocusableImage(new Image<float>(la));		\
	}								\
}

FocusableImage	FocusableImageConverterImpl::get_rgb(ImagePtr image) {
	rgb_to_focusable(image, unsigned char);
	rgb_to_focusable(image, unsigned short);
	rgb_to_focusable(image, unsigned int);
	rgb_to_focusable(image, unsigned long);
	rgb_to_focusable(image, float);
	rgb_to_focusable(image, double);
	throw WrongImageType();
}

FocusableImage	FocusableImageConverterImpl::operator()(const ImagePtr image) {
	// handle Bayer images
	try {
		return get_bayer(image);
	} catch (const WrongImageType& x) {
	}

	// handle raw images
	try {
		return get_raw(image);
	} catch (const WrongImageType& x) {
	}

	// handle YUV images
	try {
		return get_yuv(image);
	} catch (const WrongImageType& x) {
	}

	// handle RGB images
	try {
		return get_rgb(image);
	} catch (const WrongImageType& x) {
	}

	// unknown image type
	std::string	imagetype = demangle(typeid(*image).name());
	std::string	msg = stringprintf("cannot extract focusable image "
		"from %s", imagetype.c_str());
	throw std::runtime_error(msg);
}

//////////////////////////////////////////////////////////////////////
// Implementation of the parent class
//////////////////////////////////////////////////////////////////////
FocusableImageConverterPtr	FocusableImageConverter::get() {
	return FocusableImageConverterPtr(new FocusableImageConverterImpl());
}

} // namespace focusing
} // namespace astro
