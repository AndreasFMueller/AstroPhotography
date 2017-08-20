/*
 * duplicate.cpp -- duplicate an iamge
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroImage.h>
#include <AstroUtils.h>
#include <AstroDebug.h>

namespace astro {
namespace image {
namespace ops {

#define duplicate_pixel(Pixel, image)					\
{									\
	Image<Pixel>	*from = dynamic_cast<Image<Pixel >*>(&*image);	\
	if (from) {							\
		return ImagePtr(new Image<Pixel >(*from));		\
	}								\
}

static ImagePtr	duplicate_image(ImagePtr image) {
	duplicate_pixel(unsigned char, image);
	duplicate_pixel(unsigned short, image);
	duplicate_pixel(unsigned int, image);
	duplicate_pixel(unsigned long, image);
	duplicate_pixel(float, image);
	duplicate_pixel(double, image);
	duplicate_pixel(RGB<unsigned char>, image);
	duplicate_pixel(RGB<unsigned short>, image);
	duplicate_pixel(RGB<unsigned int>, image);
	duplicate_pixel(RGB<unsigned long>, image);
	duplicate_pixel(RGB<float>, image);
	duplicate_pixel(RGB<double>, image);
	duplicate_pixel(YUYV<unsigned char>, image);
	duplicate_pixel(YUYV<unsigned short>, image);
	duplicate_pixel(YUYV<unsigned int>, image);
	duplicate_pixel(YUYV<unsigned long>, image);
	duplicate_pixel(YUYV<float>, image);
	duplicate_pixel(YUYV<double>, image);
	std::string	msg = stringprintf("unknown pixel type: %s",
		demangle(typeid(image->pixel_type()).name()).c_str());
	throw std::runtime_error("unknown pixel type");
}

ImagePtr	duplicate(ImagePtr image) {
	ImagePtr	result = duplicate_image(image);
	// copy the metadata
	result->metadata(image->metadata());
	if (result->hasMetadata(std::string("UUID"))) {
		result->removeMetadata(std::string("UUID"));
	}
	// that's it
	return result;
}

} // namespace ops
} // namespace image
} // namespace astro
