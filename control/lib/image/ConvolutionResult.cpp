/*
 * ConvolutionResult.cpp -- implementation of the convolution result class
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConvolve.h>
#include <AstroTransform.h>

namespace astro {
namespace image {

ConvolutionResult::ConvolutionResult(const ImageSize& size,
	const Point& center)
	: FourierImage(size), _center(center) {
}

ConvolutionResult::ConvolutionResult(const Image<double>& image,
	const Point& center)
	: FourierImage(image), _center(center) {
}

ConvolutionResult::ConvolutionResult(const ImagePtr image,
	const Point& center)
	: FourierImage(image), _center(center) {
}

ImagePtr	ConvolutionResult::image() const {
	ImagePtr	in = inverse();
	Image<double>	*inp = dynamic_cast<Image<double>*>(&*in);
	if (NULL == inp) {
		throw std::logic_error("inverse did not return double image");
	}
	transform::RollAdapter<double>	roll(*inp, -center());
	Image<double>	*result = new Image<double>(roll);
	return ImagePtr(result);
}

ConvolutionResultPtr	operator*(const ConvolutionResultPtr a,
				const ConvolutionResultPtr b) {
	ConvolutionResult	*result = new ConvolutionResult(a->orig(),
		a->center() + b->center());
	adapter::MultiplyAdapter<double>	multi(*a, *b);
	copy(*result, multi);
	return ConvolutionResultPtr(result);
}

} // namespace image
} // namespace astro
