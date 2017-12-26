/*
 * Rescale.cpp -- postprocessing class for pixel rescaling
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroPostprocessing.h>
#include <AstroAdapter.h>
#include <AstroFilter.h>

namespace astro {
namespace image {
namespace post {

using namespace astro::adapter;
using namespace astro::image::filter;

/**
 * \brief Construct a Rescale object
 */
Rescale::Rescale() : _minimum(-1), _maximum(-1), _scale(-1) {
}

template<typename Pixel>
static ImagePtr	rescale(const ConstImageAdapter<Pixel>& image, const Rescale& r) {
	double	minimum = r.minimum();
	double	maximum = r.maximum();
	double	scale = r.scale();
	LuminanceAdapter<Pixel, double>	luminance(image);
	if (minimum < 0) {
		minimum = Min<double, double>()(luminance);
	}
	if (scale < 0) {
		if (maximum < 0) {
			maximum = Max<double, double>()(luminance);
		}
		scale = 255. / (maximum - minimum);
	}

	// create image from rescaling adapter
	Pixel	zero(minimum);
	RescalingAdapter<Pixel>	ra(image, zero, scale);
	Image<Pixel>	*rescaledimage = new Image<Pixel>(ra);
	ImagePtr	outimage(rescaledimage);
	return outimage;
}

#define	do_rescale(image, Pixel)					\
{									\
	Image<Pixel >	*imagep						\
		= dynamic_cast<Image<Pixel >*>(&*image);		\
	if (NULL != imagep) {						\
		return rescale(*imagep, *this);				\
	}								\
}

/**
 * \brief Operator to rescale an image
 */
ImagePtr	Rescale::operator()(ImagePtr image) const {
	do_rescale(image, unsigned char)
	do_rescale(image, unsigned short)
	do_rescale(image, unsigned int)
	do_rescale(image, unsigned long)
	do_rescale(image, float)
	do_rescale(image, double)
	do_rescale(image, RGB<unsigned char>)
	do_rescale(image, RGB<unsigned short>)
	do_rescale(image, RGB<unsigned int>)
	do_rescale(image, RGB<unsigned long>)
	do_rescale(image, RGB<float>)
	do_rescale(image, RGB<double>)
	throw std::runtime_error("cannot rescale this pixel type");
}

} // namespace post
} // namespace image
} // namespace astro
