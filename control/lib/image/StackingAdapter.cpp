/*
 * StackingAdapter.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroAdapter.h>
#include <AstroPixel.h>

namespace astro {
namespace adapter {

//////////////////////////////////////////////////////////////////////
// Auxiliary template derived from the Stacking
//////////////////////////////////////////////////////////////////////
/**
 * \brief Derived classes of the StackingAdapter for each pixel type
 *
 * When requesting a StackingAdapter, an instance of this template is
 * constructed
 */
template<typename Pixel>
class StackingAdapterTyped : public StackingAdapter {
	const Image<Pixel>	*_image;
	StackingAdapterTyped(const StackingAdapterTyped& other);
	StackingAdapterTyped&	operator=(const StackingAdapterTyped& other);
public:
	StackingAdapterTyped(const ImagePtr imageptr, const Image<Pixel> *image)
		: StackingAdapter(imageptr), _image(image) {
	}
	virtual ~StackingAdapterTyped() {
	}
	double	pixel(int x, int y) const {
		double	value = image::luminance<Pixel>(_image->pixel(x, y));
		return value;
	}
};

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////
#define buildadapter(imageptr, Pixel)					\
{									\
	Image<Pixel>	*image = dynamic_cast<Image<Pixel > *>(&*imageptr); \
	if (NULL != image) {						\
		return new StackingAdapterTyped<Pixel >(imageptr, image);\
	}								\
}

StackingAdapter	*StackingAdapter::get(ImagePtr imageptr) {
	buildadapter(imageptr, unsigned char);
	buildadapter(imageptr, unsigned short);
	buildadapter(imageptr, unsigned int);
	buildadapter(imageptr, unsigned long);
	buildadapter(imageptr, float);
	buildadapter(imageptr, double);
	buildadapter(imageptr, RGB<unsigned char>);
	buildadapter(imageptr, RGB<unsigned short>);
	buildadapter(imageptr, RGB<unsigned int>);
	buildadapter(imageptr, RGB<unsigned long>);
	buildadapter(imageptr, RGB<float>);
	buildadapter(imageptr, RGB<double>);
	buildadapter(imageptr, YUYV<unsigned char>);
	buildadapter(imageptr, YUYV<unsigned short>);
	buildadapter(imageptr, YUYV<unsigned int>);
	buildadapter(imageptr, YUYV<unsigned long>);
	buildadapter(imageptr, YUYV<float>);
	buildadapter(imageptr, YUYV<double>);
	throw std::runtime_error("unknown pixel type");
}

} // namespace adapter
} // namespace asro

