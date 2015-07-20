/*
 * DisplayConverter.cpp -- Display converter implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDisplay.h>
#include <AstroAdapter.h>
#include <AstroTransform.h>
#include <AstroDebug.h>

using namespace astro::image::transform;
using namespace astro::adapter;

namespace astro {
namespace image {

DisplayConverter::DisplayConverter() {
	scale = 1;
	minpixel = 0;
	maxpixel = 0;
	color = false;
}

void	DisplayConverter::setSubframe(const ImageRectangle& _subframe) {
	subframe = _subframe;
}

ImageRectangle  DisplayConverter::getSubframe() const {
	return subframe;
}

void    DisplayConverter::setScale(double _scale) {
	scale = _scale;
}

double  DisplayConverter::getScale() const {
	return scale;
}

void    DisplayConverter::setMinpixel(double _minpixel) {
	minpixel = _minpixel;
}

double  DisplayConverter::getMinpixel() const {
	return minpixel;
}

void    DisplayConverter::setMaxpixel(double _maxpixel) {
	maxpixel = _maxpixel;
}

double  DisplayConverter::getMaxpixel() const {
	return maxpixel;
}

void    DisplayConverter::setColor(bool _color) {
	color = _color;
}

bool    DisplayConverter::getColor() const {
	return color;
}

#define	convertColor_typed(Pixel)					\
{									\
	Image<Pixel>	*imageptr					\
		= dynamic_cast<Image<Pixel > *>(&*image);		\
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%p", imageptr);			\
	if (NULL != imageptr) {						\
		ConvertingAdapter<RGB<double>, Pixel >	ca(*imageptr);	\
		colorimage = new Image<RGB<double> >(ca);		\
	}								\
}

#define convertColor_yuyv(Pixel)					\
{									\
	Image<YUYV<Pixel> >	*imageptr				\
		= dynamic_cast<Image<YUYV<Pixel> > *>(&*image);		\
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%p", imageptr);			\
	if (NULL != imageptr) {						\
		YUYVAdapter<Pixel>	ya(*imageptr);			\
		ConvertingAdapter<RGB<double>, RGB<Pixel > >	ca(ya);	\
		colorimage = new Image<RGB<double> >(ca);		\
	}								\
}

Image<RGB<unsigned char> >	*DisplayConverter::convertColor(const ImagePtr image) {
	// make sure take the right scaling
	ImageRectangle	frame = subframe;
	if (frame.size().width() == 0) {
		frame.setOrigin(ImagePoint(0, 0));
		frame.setSize(image->size());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "extracting color subframe %s",
		frame.toString().c_str());

	// convert the image to a RGB<double>
	Image<RGB<double> >	*colorimage = NULL;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "trying primitive pixel types");
	convertColor_typed(unsigned char);
	convertColor_typed(unsigned short);
	convertColor_typed(unsigned int);
	convertColor_typed(unsigned long);
	convertColor_typed(float);
	convertColor_typed(double);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "trying RGB pixel types");
	convertColor_typed(RGB<unsigned char>);
	convertColor_typed(RGB<unsigned short>);
	convertColor_typed(RGB<unsigned int>);
	convertColor_typed(RGB<unsigned long>);
	convertColor_typed(RGB<float>);
	convertColor_typed(RGB<double>);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "trying YUYV pixel types");
	convertColor_yuyv(unsigned char);
	convertColor_yuyv(unsigned short);
	convertColor_yuyv(unsigned int);
	convertColor_yuyv(unsigned long);
	convertColor_yuyv(float);
	convertColor_yuyv(double);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "color subframe of size %s extracted",
		colorimage->size().toString().c_str());

	// now rescale the image to acceptable values
	double	pixelscale = 255.5 / (maxpixel - minpixel);
	RescalingAdapter<RGB<double> >	ra(*colorimage, minpixel, pixelscale);

	// extract the subwindow we want to see
	WindowAdapter<RGB<double> >	wa(ra, frame);

	// apply the scaling transformation
	Transform	transform(0, Point(0, 0), scale);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "transform: %s",
		transform.toString().c_str());
	ImageSize	targetsize(wa.getSize().width() * scale,
				wa.getSize().height() * scale);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "target size: %s",
		targetsize.toString().c_str());
	TransformAdapter<RGB<double> >	ta(targetsize, wa, transform);

	// convert to an RGB image
	ConvertingAdapter<RGB<unsigned char>, RGB<double> >	ca(ta);
	//ConvertingAdapter<RGB<unsigned char>, RGB<double> >	ca(*colorimage);

	Image<RGB<unsigned char> >	*result
		= new Image<RGB<unsigned char> >(ca);

	delete colorimage;
	return result;
}

#define	convertBW_typed(Pixel)						\
{									\
	Image<Pixel>	*imageptr					\
		= dynamic_cast<Image<Pixel > *>(&*image);		\
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tryping type: %p", imageptr);	\
	if (NULL != imageptr) {						\
		LuminanceAdapter<Pixel, double>	la(*imageptr);		\
		luminanceimage = new Image<double>(la);			\
	}								\
}

Image<RGB<unsigned char> >	*DisplayConverter::convertBW(const ImagePtr image) {
	// make sure take the right scaling
	ImageRectangle	frame = subframe;
	if (frame.size().width() == 0) {
		frame.setOrigin(ImagePoint(0, 0));
		frame.setSize(image->size());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "extracting luminance image");

	// convert the image to a Luminance only image
	Image<double>	*luminanceimage = NULL;

	// try each possible type
	convertBW_typed(unsigned char);
	convertBW_typed(unsigned short);
	convertBW_typed(unsigned int);
	convertBW_typed(unsigned long);
	convertBW_typed(float);
	convertBW_typed(double);
	convertBW_typed(RGB<unsigned char>);
	convertBW_typed(RGB<unsigned short>);
	convertBW_typed(RGB<unsigned int>);
	convertBW_typed(RGB<unsigned long>);
	convertBW_typed(RGB<float>);
	convertBW_typed(RGB<double>);
	convertBW_typed(YUYV<unsigned char>);
	convertBW_typed(YUYV<unsigned short>);
	convertBW_typed(YUYV<unsigned int>);
	convertBW_typed(YUYV<unsigned long>);
	convertBW_typed(YUYV<float>);
	convertBW_typed(YUYV<double>);
	if (NULL == luminanceimage) {
		throw std::runtime_error("failed to convert image to luminance "
			"only");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "converted to luminance");

	// now rescale the luminance values
	double	pixelscale = 255.5 / (maxpixel - minpixel);
	RescalingAdapter<double>	ra(*luminanceimage,
						minpixel, pixelscale);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rescaling luminance values using %f [%f, %f]",
		pixelscale, minpixel, maxpixel);

	// extract the subwindow we want to see
	WindowAdapter<double>	wa(ra, frame);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "subwindow adapter: %s",
		frame.toString().c_str());

	// apply the scaling transformation
	Transform	transform(0, Point(0, 0), scale);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "transform: %s",
		transform.toString().c_str());
	ImageSize	targetsize(wa.getSize().width() * scale,
				wa.getSize().height() * scale);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "target size: %s",
		targetsize.toString().c_str());
	TransformAdapter<double>	ta(targetsize, wa, transform);

	// convert to an RGB image
	ConvertingAdapter<RGB<unsigned char>, double>	ca(ta);

	Image<RGB<unsigned char> >	*result
		= new Image<RGB<unsigned char> >(ca);

	delete luminanceimage;
	return result;
}

Image<RGB<unsigned char> >      *DisplayConverter::operator()(const ImagePtr image) {
	if (color) {
		return convertColor(image);
	} else {
		return convertBW(image);
	}
}

} // namespace image
} // namespace astro
