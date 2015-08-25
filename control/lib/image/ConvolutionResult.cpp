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

class ComplexMultiplyAdapter : public ConstImageAdapter<double> {
	const ConstImageAdapter<double>&	_op1;
	const ConstImageAdapter<double>&	_op2;
public:
	ComplexMultiplyAdapter(const ConstImageAdapter<double>& operand1,
		const ConstImageAdapter<double>& operand2)
		: ConstImageAdapter<double>(operand1.getSize()),
		  _op1(operand1), _op2(operand2) {
	}
	virtual double	pixel(int x, int y) const {
		int	t = x % 2;
		int	x0 = x - t;
		double	v = 0;
		if (0 == t) {
			v = _op1.pixel(x0, y) * _op2.pixel(x0, y) -
				_op1.pixel(x0 + 1, y) * _op2.pixel(x0 + 1, y);
		} else {
			v = _op1.pixel(x0, y) * _op2.pixel(x0 + 1, y) +
				_op1.pixel(x0 + 1, y) * _op2.pixel(x0, y);

		}
		return v;
	}
};

ConvolutionResultPtr	operator*(const ConvolutionResultPtr a,
				const ConvolutionResultPtr b) {
	ConvolutionResult	*result = new ConvolutionResult(a->orig(),
		a->center() + b->center());
	ComplexMultiplyAdapter	multi(*a, *b);
	copy(*result, multi);
	return ConvolutionResultPtr(result);
}

ConvolutionResultPtr	operator*(const ConvolutionResult& a,
				const ConvolutionResult& b) {
	ConvolutionResult	*result = new ConvolutionResult(a.orig(),
		a.center() + b.center());
	ComplexMultiplyAdapter	multi(a, b);
	copy(*result, multi);
	return ConvolutionResultPtr(result);
	
}

} // namespace image
} // namespace astro
