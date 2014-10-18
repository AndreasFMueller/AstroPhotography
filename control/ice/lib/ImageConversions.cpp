/*
 * ImageConversions.cpp
 *
 * (c) 2014 Prof Dr Andreas Mueller, 
 */
#include <IceConversions.h>
#include <type_traits>
#include <limits>

namespace snowstar {

/**
 * \brief Convert a simple image to a astro::image::Image
 */
astro::image::ImagePtr	convertsimple(SimpleImage image) {
	astro::image::ImageSize	size(image.size.width, image.size.height);
	astro::image::Image<unsigned short>	*imageptr
		= new astro::image::Image<unsigned short>(size);
	long	length = image.size.width * image.size.height;
	for (int offset = 0; offset < length; offset++) {
		(*imageptr)[offset] = image.imagedata[offset];
	}
	return astro::image::ImagePtr(imageptr);
}

/**
 * \brief An adapter template class to convert to unsigned short pixels
 */
template<typename Pixel>
class UnsignedShortAdapter
	: public astro::image::ConstImageAdapter<unsigned short> {
	const astro::image::ConstImageAdapter<Pixel>&	image;
public:
	UnsignedShortAdapter(const astro::image::ConstImageAdapter<Pixel>&
		_image);
	virtual unsigned short	pixel(unsigned int x, unsigned int y) const;
};

/**
 * \brief Constructor for the Unsigned short adapter
 */
template<typename Pixel>
UnsignedShortAdapter<Pixel>::UnsignedShortAdapter(
			const astro::image::ConstImageAdapter<Pixel>& _image)
	: astro::image::ConstImageAdapter<unsigned short>(_image.getSize()),
	  image(_image) {
}

/**
 * \brief Pixel accessor for the unsigned short adapter
 *
 */
template<typename Pixel>
unsigned short	UnsignedShortAdapter<Pixel>::pixel(unsigned int x,
		unsigned int y) const {
	unsigned short	value;
	// code to be compiled for floating point pixel types
	if (std::is_floating_point<Pixel>()) {
		value = std::numeric_limits<unsigned short>::max()
			* image.pixel(x, y);
	}
	// code to be compiled for integral pixel types
	if (std::is_integral<Pixel>()) {
		// if the pixel has more pixels that unsigned short...
		if (std::numeric_limits<Pixel>::digits > 16) {
			// ... then we shift the value to the right and ..
			value = image.pixel(x, y) >>
				(std::numeric_limits<Pixel>::digits - 16);
		} else {
			// ... otherwise we shift to the left
			value = image.pixel(x, y) << 
				(16 - std::numeric_limits<Pixel>::digits);
		}
	}
	return value;
}

#define	get_reduction(Pixel)						\
{									\
	astro::image::Image<Pixel >	*dimage				\
		= dynamic_cast<astro::image::Image<Pixel > *>(&*image);	\
	if (NULL != dimage) {						\
		reduction = new UnsignedShortAdapter<Pixel >(*dimage);	\
	}								\
}


/**
 * \brief Convert an astro::image::Image to a SimpleImage
 */
SimpleImage	convertsimple(astro::image::ImagePtr image) {
	SimpleImage	result;

	// convert the size
	result.size = convert(image->size());

	// try to convert directly
	astro::image::Image<unsigned short>	*im
		= dynamic_cast<astro::image::Image<unsigned short> *>(&*image);
	if (NULL != im) {
		for (int x = 0; x < result.size.width; x++) {
			for (int y = 0; y < result.size.width; y++) {
				unsigned short	value = im->pixel(x, y);
				result.imagedata.push_back(value);
			}
		}
		return result;
	}

	// create an image adapter that allows us to read unsigned short
	// values from the image
	astro::image::ConstImageAdapter<unsigned short>	*reduction = NULL;
	get_reduction(unsigned char)
	get_reduction(unsigned long)

	// convert the image data
	if (NULL == reduction) {
		throw std::runtime_error("no reduction found");
	}

	// if we get to this point, then a suitable reduction exists and
	// we can use it to convert pixel values to unsigned short
	for (int x = 0; x < result.size.width; x++) {
		for (int y = 0; y < result.size.width; y++) {
			unsigned short	value = reduction->pixel(x, y);
			result.imagedata.push_back(value);
		}
	}

	return result;
}



} // namespace snowstar
