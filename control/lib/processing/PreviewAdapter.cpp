/*
 * PreviewAdapter.cpp -- implementation of the preview adapter
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <AstroFilter.h>

using namespace astro::image::filter;

namespace astro {
namespace adapter {

/**
 * \brief Map a pixel value to the range of unsigned char
 */
template<typename Pixel>
unsigned char	map_pixel_value(double _min, double _max, Pixel value) {
	double	v = 255 * (value - _min) / (_max - _min);
	if (v < 0) { return 0; }
	if (v > 255) { return 255; }
	return (unsigned char)v;
}

/**
* \brief Template function and specializations to determine the maximum value
 */
template<typename Pixel>
static double	pixel_max_value() {
	return 1.;
}

template<>
double	pixel_max_value<unsigned char>() { return 255.; }
template<>
double	pixel_max_value<unsigned short>() { return 65535.; }
template<>
double	pixel_max_value<unsigned int>() { return 4294967295.; }
template<>
double	pixel_max_value<unsigned long>() { return 4294967295.; }

//////////////////////////////////////////////////////////////////////
// PreviewAdapter template for monochrome images
//////////////////////////////////////////////////////////////////////

/**
 * \brief preview adapter tempate for monochrome images
 */
template<typename Pixel>
class TypedImagePreviewAdapter : public PreviewAdapter {
	const Image<Pixel>	*_image;
public:
	TypedImagePreviewAdapter(const Image<Pixel> *image) : _image(image) {
		_max = pixel_max_value<Pixel>();
	}

	ImageSize	size() const { return _image->size(); }

	/**
	 * \brief map monochrome pixels to unsigned char range
	 */
	virtual unsigned char	monochrome_pixel(int x, int y) const {
		return map_pixel_value(_min, _max, _image->pixel(x, y));
	}

	/**
 	 * \brief map monochrome RGB pixels with unsigned char range
	 */
	virtual RGB<unsigned char>	color_pixel(int x, int y) const {
		return RGB<unsigned char>(monochrome_pixel(x, y));
	}
};

//////////////////////////////////////////////////////////////////////
// PreviewAdapter template for color images
//////////////////////////////////////////////////////////////////////

/**
 * \brief preview adapter template for color images
 */
template<typename Pixel>
class TypedRGBImagePreviewAdapter : public PreviewAdapter {
	const Image<RGB<Pixel> >	*_image;
public:
	TypedRGBImagePreviewAdapter(const Image<RGB<Pixel> > *image)
		: _image(image) {
		_max = pixel_max_value<Pixel>();
	}

	ImageSize	size() const { return _image->size(); }

	/**
	 * \brief Create monochrome pixels from luminance
	 */
	virtual unsigned char	monochrome_pixel(int x, int y) const {
		Pixel	p = _image->pixel(x, y).luminance();
		return map_pixel_value(_min, _max, p);
	}

	/**
	 * \brief limit a color pixel to unsigned char range
	 */
	virtual RGB<unsigned char>	color_pixel(int x, int y) const {
		RGB<Pixel>	p = _image->pixel(x, y);
		return RGB<unsigned char>(
			map_pixel_value(_min, _max, p.R),
			map_pixel_value(_min, _max, p.G),
			map_pixel_value(_min, _max, p.B));
	}
};

//////////////////////////////////////////////////////////////////////
// PreviewAdapter implementation
//////////////////////////////////////////////////////////////////////
/**
 * \brief Destructor for the base PreviewAdapter
 */
PreviewAdapter::~PreviewAdapter() {
}

#define	monochrome_preview_adapter(image, Pixel)			\
{									\
	const Image<Pixel>	*p					\
		= dynamic_cast<const Image<Pixel> *>(image);		\
	if (NULL != p) {						\
		Max<Pixel, double>	max;				\
		double	maxvalue = max.filter(*p);			\
		Min<Pixel, double>	min;				\
		double	minvalue = min.filter(*p);			\
		if (minvalue == maxvalue) {				\
			maxvalue = minvalue + 1;			\
		}							\
		PreviewAdapterPtr	preview				\
			= PreviewAdapterPtr(				\
				new TypedImagePreviewAdapter<Pixel>(p));\
		preview->min(minvalue);					\
		preview->min(maxvalue);					\
		return preview;						\
	}								\
}

#define	color_preview_adapter(image, Pixel)				\
{									\
	const Image<RGB<Pixel> >	*p				\
		= dynamic_cast<const Image<RGB<Pixel> > *>(image);	\
	if (NULL != p) {						\
		LuminanceAdapter<RGB<Pixel>, double>	l(*p);		\
		Max<double, double>	max;				\
		double	maxvalue = max.filter(l);			\
		Min<double, double>	min;				\
		double	minvalue = min.filter(l);			\
		if (minvalue == maxvalue) {				\
			maxvalue = minvalue + 1;			\
		}							\
		PreviewAdapterPtr	preview				\
			= PreviewAdapterPtr(				\
				new TypedRGBImagePreviewAdapter<Pixel>(p));\
		preview->min(minvalue);					\
		preview->min(maxvalue);					\
		return preview;						\
	}								\
}


PreviewAdapterPtr	PreviewAdapter::get(const ImageBase *image) {
	monochrome_preview_adapter(image, unsigned char);
	monochrome_preview_adapter(image, unsigned short);
	monochrome_preview_adapter(image, unsigned int);
	monochrome_preview_adapter(image, unsigned long);
	monochrome_preview_adapter(image, float);
	monochrome_preview_adapter(image, double);
	color_preview_adapter(image, unsigned char);
	color_preview_adapter(image, unsigned short);
	color_preview_adapter(image, unsigned int);
	color_preview_adapter(image, unsigned long);
	color_preview_adapter(image, float);
	color_preview_adapter(image, double);
	throw std::runtime_error("cannot preview this image");
}

PreviewAdapterPtr	PreviewAdapter::get(ImagePtr image) {
	monochrome_preview_adapter(&*image, unsigned char);
	monochrome_preview_adapter(&*image, unsigned short);
	monochrome_preview_adapter(&*image, unsigned int);
	monochrome_preview_adapter(&*image, unsigned long);
	monochrome_preview_adapter(&*image, float);
	monochrome_preview_adapter(&*image, double);
	color_preview_adapter(&*image, unsigned char);
	color_preview_adapter(&*image, unsigned short);
	color_preview_adapter(&*image, unsigned int);
	color_preview_adapter(&*image, unsigned long);
	color_preview_adapter(&*image, float);
	color_preview_adapter(&*image, double);
	throw std::runtime_error("cannot preview this image");
}

} // namespace adapter
} // namespace astro
