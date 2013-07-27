/*
 * AstroAdapter.h -- a collection of adapters
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroImage.h>
#include <AstroMask.h>
#include <AstroDebug.h>

namespace astro {
namespace image {

//////////////////////////////////////////////////////////////////////
// Accessing Subrectangles of an Image
//////////////////////////////////////////////////////////////////////

/**
 * \brief Adapter for a subimage
 *
 * This adapter allows to treat a subrectangle of an image just as if it
 * were the image itself, except that the image cannot be changed.
 */
template<typename Pixel>
class WindowAdapter : public ConstImageAdapter<Pixel> {
	const ConstImageAdapter<Pixel>&	image;
	ImageRectangle	frame;
public:
	WindowAdapter(const ConstImageAdapter<Pixel>& image, const ImageRectangle& frame);
	
	virtual const Pixel	pixel(unsigned int x, unsigned int y) const;
};

/**
 * \brief Construct a subimage adapter
 */
template<typename Pixel>
WindowAdapter<Pixel>::WindowAdapter(const ConstImageAdapter<Pixel>& _image,
	const ImageRectangle& _frame)
	: ConstImageAdapter<Pixel>(_frame.size),
	  image(_image), frame(_frame) {
}

/**
 * \bief Access pixel inside the subwindow
 */
template<typename Pixel>
const Pixel	WindowAdapter<Pixel>::pixel(unsigned int x, unsigned int y) const {
	return	image.pixel(frame.origin.x + x, frame.origin.y + y);
}

//////////////////////////////////////////////////////////////////////
// Converting Pixel values
//////////////////////////////////////////////////////////////////////
/**
 * \brief Adapter to subimage with implied pixel type conversion
 */
template<typename TargetPixel, typename SourcePixel>
class ConvertingAdapter : public ConstImageAdapter<TargetPixel> {
	const ConstImageAdapter<SourcePixel>&	 image;
public:
	ConvertingAdapter(const ConstImageAdapter<SourcePixel>& image);
	virtual const TargetPixel	pixel(unsigned int x, unsigned int y) const;
};

template<typename TargetPixel, typename SourcePixel>
ConvertingAdapter<TargetPixel, SourcePixel>::ConvertingAdapter(
	const ConstImageAdapter<SourcePixel>& _image)
	: ConstImageAdapter<TargetPixel>(_image.getSize()), image(_image) {
}

template<typename TargetPixel, typename SourcePixel>
const TargetPixel	ConvertingAdapter<TargetPixel, SourcePixel>::pixel(unsigned int x, unsigned int y) const {
	const SourcePixel	t = image.pixel(x, y);
	// convert to Pixel type
	TargetPixel	p(t);
	return p;
}

//////////////////////////////////////////////////////////////////////
// Adapter for access to a subgrid
//////////////////////////////////////////////////////////////////////

/**
 * \brief Adapter to a subgrid
 */
template<typename Pixel>
class ConstSubgridAdapter : public ConstImageAdapter<Pixel> {
	const ConstImageAdapter<Pixel>&	image;
	Subgrid	subgrid;
public:
	ConstSubgridAdapter(const ConstImageAdapter<Pixel>& image,
		const Subgrid& subgrid);
	virtual const Pixel	pixel(unsigned int x, unsigned int y) const;
};

template<typename Pixel>
ConstSubgridAdapter<Pixel>::ConstSubgridAdapter(
	const ConstImageAdapter<Pixel>& _image, const Subgrid& _subgrid)
	: ConstImageAdapter<Pixel>(ImageSize(
		(_image.getSize().getWidth() - _subgrid.origin.x)
			/ _subgrid.stepsize.getWidth(),
		(_image.getSize().getHeight() - _subgrid.origin.y)
			/ _subgrid.stepsize.getHeight())
	), image(_image), subgrid(_subgrid) {
}
	
template<typename Pixel>
const Pixel	ConstSubgridAdapter<Pixel>::pixel(unsigned int x,
			unsigned int y) const {
	return image.pixel(subgrid.x(x), subgrid.y(y));
}

/**
 * \brief Mutable adapter to a subgrid
 */
template<typename Pixel>
class SubgridAdapter : public ImageAdapter<Pixel> {
	ImageAdapter<Pixel>&	image;
	Subgrid	subgrid;
public:
	SubgridAdapter(ImageAdapter<Pixel>& image,
		const Subgrid& subgrid);
	virtual const Pixel	pixel(unsigned int x, unsigned int y) const;
	virtual Pixel&	pixel(unsigned int x, unsigned int y);
};

template<typename Pixel>
SubgridAdapter<Pixel>::SubgridAdapter(
	ImageAdapter<Pixel>& _image, const Subgrid& _subgrid)
	: ImageAdapter<Pixel>(ImageSize(
		(_image.getSize().getWidth() - _subgrid.origin.x)
			/ _subgrid.stepsize.getWidth(),
		(_image.getSize().getHeight() - _subgrid.origin.y)
			/ _subgrid.stepsize.getHeight())
	), image(_image), subgrid(_subgrid) {
}
	
template<typename Pixel>
const Pixel	SubgridAdapter<Pixel>::pixel(unsigned int x,
			unsigned int y) const {
	return image.pixel(subgrid.x(x), subgrid.y(y));
}

template<typename Pixel>
Pixel&	SubgridAdapter<Pixel>::pixel(unsigned int x, unsigned int y) {
	return image.pixel(subgrid.x(x), subgrid.y(y));
}

//////////////////////////////////////////////////////////////////////
// Adapter for arithmetic operations
//////////////////////////////////////////////////////////////////////

/**
 * \brief Base class for arithmetic operation adapters
 */ 
template<typename Pixel>
class ArithmeticAdapter : public ConstImageAdapter<double> {
protected:
	const ConstImageAdapter<Pixel>&	operand1;
	const ConstImageAdapter<Pixel>&	operand2;
public:
	ArithmeticAdapter(const ConstImageAdapter<Pixel>& summand1,
		const ConstImageAdapter<Pixel>& summand2);
};

/**
 * \brief Constructor
 *
 * The constructor verifies that the two operands have the same size
 */
template<typename Pixel>
ArithmeticAdapter<Pixel>::ArithmeticAdapter(
	const ConstImageAdapter<Pixel>& _operand1,
	const ConstImageAdapter<Pixel>& _operand2)
	: ConstImageAdapter<Pixel>(_operand1.getSize()),
	  operand1(_operand1), operand2(_operand2) {
	// verify that the two operands have the same size
	if (operand1.getSize() != operand2.getSize()) {
		throw std::runtime_error("summand size does not match");
	}
}

/**
 * \brief Add adapter
 *
 * Can be used to add two Images
 */
template<typename Pixel>
class AddAdapter : public ArithmeticAdapter<Pixel> {
public:
	AddAdapter(const ConstImageAdapter<Pixel>& summand1,
		const ConstImageAdapter<Pixel>& summand2);
	virtual const double	pixel(unsigned int x, unsigned int y) const;
};

/**
 * \brief Construct an addition adapter
 */
template<typename Pixel>
AddAdapter<Pixel>::AddAdapter(const ConstImageAdapter<Pixel>& summand1,
		const ConstImageAdapter<Pixel>& summand2)
	: ArithmeticAdapter<Pixel>(summand1, summand2) {
}

/**
 * \brief Perform the addition
 */
template<typename Pixel>
const double	AddAdapter<Pixel>::pixel(unsigned int x, unsigned int y) const {
	double	result = 0;
	result += ArithmeticAdapter<Pixel>::operand1.pixel(x, y);
	result += ArithmeticAdapter<Pixel>::operand2.pixel(x, y);
	return result;
}

template<typename Pixel>
class MultiplyAdapter : public ArithmeticAdapter<Pixel> {
public:
	MultiplyAdapter(const ConstImageAdapter<Pixel>& summand1,
		const ConstImageAdapter<Pixel>& summand2);
	virtual const double	pixel(unsigned int x, unsigned int y) const;
};

template<typename Pixel>
MultiplyAdapter<Pixel>::MultiplyAdapter(
		const ConstImageAdapter<Pixel>& operand1,
		const ConstImageAdapter<Pixel>& operand2)
	: ArithmeticAdapter<Pixel>(operand1, operand2) {
}

template<typename Pixel>
const double	MultiplyAdapter<Pixel>::pixel(unsigned int x, unsigned int y) const {
	double	result = 1;
	result *= ArithmeticAdapter<Pixel>::operand1.pixel(x, y);
	result *= ArithmeticAdapter<Pixel>::operand2.pixel(x, y);
	return result;
}

//////////////////////////////////////////////////////////////////////
// Adapter to compute the Laplacian of an image
//////////////////////////////////////////////////////////////////////
/**
 * \brief Adapter that computes the image Laplacian
 *
 * The Laplacian is used to compute a figure of merit for the focus of an image.
 * There the value if the laplacian is multiplied with the image value
 * at the same point, and everything is integrated. 
 */
template<typename Pixel>
class LaplacianAdapter : public ConstImageAdapter<double> {
	const ConstImageAdapter<Pixel>& image;
	bool	diagonal;
	double	scale;
public:
	LaplacianAdapter(const ConstImageAdapter<Pixel>& image,
		bool diagonal = false);
	const double	pixel(unsigned int x, unsigned int y) const;
};

/**
 * \brief Construct a Laplacian Adapter
 */
template<typename Pixel>
LaplacianAdapter<Pixel>::LaplacianAdapter(
	const ConstImageAdapter<Pixel>& _image, bool _diagonal)
	: ConstImageAdapter<double>(_image.getSize()),
	  image(_image), diagonal(_diagonal) {
	if (diagonal) {
		scale = sqrt(2);
	} else {
		scale = 1;
	}
}

/**
 * \brief compute the Laplacian at a given point
 *
 * Note the special treatement of the points at the boundary. In many cases
 * it will be advantegous to apply a mask to the image so that boundary
 * artefacts do not misrepresent e.g. the focus.
 */
template<typename Pixel>
const double	LaplacianAdapter<Pixel>::pixel(unsigned int x, unsigned int y)
			const {
	double	result = 0;
	int	counter;
	if (diagonal) {
		if ((x > 0) && (x < adaptersize.getWidth() - 1) &&
			(y > 0) && (y < adaptersize.getHeight() - 1)) {
			result += image.pixel(x - 1, y - 1);
			result += image.pixel(x + 1, y - 1);
			result += image.pixel(x - 1, y + 1);
			result += image.pixel(x + 1, y + 1);
			counter += 4;
		}
	} else {
		if ((x > 0) && (x < adaptersize.getWidth() - 1)) {
			result += image.pixel(x - 1, y);
			result += image.pixel(x + 1, y);
			counter += 2;
		}
		if ((y > 0) && (y < adaptersize.getHeight() - 1)) {
			result += image.pixel(x, y - 1);
			result += image.pixel(x, y + 1);
			counter += 2;
		}
	}
	if (counter == 0) {
		return 0;
	}
	double	center = image.pixel(x, y);
	result -= counter * center;
	return result / (scale * counter);
}

//////////////////////////////////////////////////////////////////////
// Focus Figure of merit adapter for 
//////////////////////////////////////////////////////////////////////
template<typename Pixel>
class FocusFOMAdapter : public ConstImageAdapter<double> {
	LaplacianAdapter<Pixel>	laplacian;
	ConvertingAdapter<double, Pixel>	converting;
	MultiplyAdapter<double>	multiply;
public:
	FocusFOMAdapter(const ConstImageAdapter<Pixel>& image,
		bool diagonal = false);
	const double	pixel(unsigned int x, unsigned int y) const;
};

template<typename Pixel>
FocusFOMAdapter<Pixel>::FocusFOMAdapter(const ConstImageAdapter<Pixel>& _image,
	bool diagonal)
	: ConstImageAdapter<double>(ImageSize(_image.getSize().getWidth() - 2,
		_image.getSize().getHeight() - 2)),
	  laplacian(_image, diagonal),
	  converting(_image),
	  multiply(laplacian, converting) {
}

template<typename Pixel>
const double FocusFOMAdapter<Pixel>::pixel(unsigned int x, unsigned int y) const {
	return -multiply.pixel(x + 1, y + 1);
}

//////////////////////////////////////////////////////////////////////
// Adapter that applies a masking function to an image
//////////////////////////////////////////////////////////////////////
template<typename Pixel>
class MaskingAdapter : public ConstImageAdapter<Pixel> {
	const ConstImageAdapter<Pixel>&	image;
	const MaskingFunction&	maskingfunction;
public:
	MaskingAdapter(const ConstImageAdapter<Pixel>& image,
		const MaskingFunction& maskingfunction);
	virtual const Pixel	pixel(unsigned int x, unsigned int y) const;
};

template<typename Pixel>
MaskingAdapter<Pixel>::MaskingAdapter(const ConstImageAdapter<Pixel>& _image,
	const MaskingFunction& _maskingfunction)
	: ConstImageAdapter<Pixel>(image.getSize()),
	  image(_image), maskingfunction(_maskingfunction) {
}

template<typename Pixel>
const Pixel	MaskingAdapter<Pixel>::pixel(unsigned int x, unsigned int y) const {
	Pixel	v = image.pixel(x, y);
	double m = maskingfunction(x, y);
	v *= m;
	return v;
}

//////////////////////////////////////////////////////////////////////
// Caching adapter
//////////////////////////////////////////////////////////////////////
template<typename Pixel>
class CachingAdapter : public ConstImageAdapter<Pixel> {
	const ConstImageAdapter<Pixel>&	image;
	bool	tags[];
	Pixel	values[];
public:
	CachingAdapter(const ConstImageAdapter<Pixel>& image);
	~CachingAdapter();
	virtual const Pixel	pixel(unsigned int x, unsigned int y) const;
};

template<typename Pixel>
CachingAdapter<Pixel>::CachingAdapter(const ConstImageAdapter<Pixel>& _image)
	: ConstImageAdapter<Pixel>(_image.getSize()), image(_image) {
	size_t	pixels = image.getSize().pixels;
	tags = new bool[pixels];
	for (unsigned int i = 0; i < pixels; i++) {
		tags[i] = false;
	}
	values = new Pixel[pixels];
}

template<typename Pixel>
CachingAdapter<Pixel>::~CachingAdapter() {
	delete[] tags;
	delete[] values;
}

template<typename Pixel>
const Pixel	CachingAdapter<Pixel>::pixel(unsigned int x, unsigned int y) const {
	// check if a cached value is available
	unsigned int	offset = ConstImageAdapter<Pixel>::adaptersize.offset(x, y);
	if (tags[offset]) {
		return values[offset];
	}
	tags[offset] = true;
	values[offset] = image.pixel(x, y);
	return values[offset];
}

//////////////////////////////////////////////////////////////////////
// Up/Downsampling adapters
//////////////////////////////////////////////////////////////////////
template<typename Pixel>
class DownSamplingAdapter : public ConstImageAdapter<Pixel> {
	const ConstImageAdapter<Pixel>&	image;
	ImageSize	sampling;
	double	*weights;
	unsigned int	volume;
public:
	DownSamplingAdapter(const ConstImageAdapter<Pixel>& image,
		const ImageSize& sampling);
	virtual	~DownSamplingAdapter();
	const Pixel	pixel(unsigned int x, unsigned int y) const;
};

template<typename Pixel>
DownSamplingAdapter<Pixel>::DownSamplingAdapter(
	const ConstImageAdapter<Pixel>& _image, const ImageSize& _sampling)
	: ConstImageAdapter<Pixel>(
		ImageSize(_image.getSize().getWidth() / _sampling.getWidth(),
			_image.getSize().getHeight() / _sampling.getHeight())),
	  image(_image), sampling(_sampling) {
	volume = sampling.getWidth() * sampling.getHeight();
	weights = new double[volume];
	weights[0] = 1./volume;
	for (unsigned int index = 0; index < volume; index++) {
		weights[index] = 1./volume;
	}
}

template<typename Pixel>
DownSamplingAdapter<Pixel>::~DownSamplingAdapter() {
	delete[] weights;
}

template<typename Pixel>
const Pixel	DownSamplingAdapter<Pixel>::pixel(unsigned int x,
	unsigned int y) const {
	unsigned int	originx = x * sampling.getWidth();
	unsigned int	originy = y * sampling.getHeight();
	Pixel	pixels[volume];
	unsigned int	index = 0;
	for (unsigned int dx = 0; dx < sampling.getWidth(); dx++) {
		for (unsigned int dy = 0; dy < sampling.getHeight(); dy++) {
			pixels[index++]
				= image.pixel(originx + dx, originy + dy);
		}
	}
	return weighted_sum(index, weights, pixels);
}

ImagePtr	downsample(ImagePtr image, const ImageSize& sampling);

template<typename Pixel>
class UpSamplingAdapter : public ConstImageAdapter<Pixel> {
	const ConstImageAdapter<Pixel>&	image;
	ImageSize	sampling;
public:
	UpSamplingAdapter(const ConstImageAdapter<Pixel>& image,
		const ImageSize& sampling);
	const Pixel	pixel(unsigned int x, unsigned int y) const;
};

template<typename Pixel>
UpSamplingAdapter<Pixel>::UpSamplingAdapter(
	const ConstImageAdapter<Pixel>& _image, const ImageSize& _sampling)
	: ConstImageAdapter<Pixel>(
		ImageSize(_image.getSize().getWidth() * _sampling.getWidth(),
			_image.getSize().getHeight() * _sampling.getHeight())),
	  image(_image), sampling(_sampling) {
}

template<typename Pixel>
const Pixel	UpSamplingAdapter<Pixel>::pixel(unsigned int x,
	unsigned int y) const {
	return image.pixel(x / sampling.getWidth(), y / sampling.getWidth());
}

ImagePtr	upsample(ImagePtr image, const ImageSize& sampling);

//////////////////////////////////////////////////////////////////////
// Luminance Adapter
//////////////////////////////////////////////////////////////////////
template<typename Pixel>
class LuminanceAdapter : public ConstImageAdapter<double> {
	const ConstImageAdapter<Pixel>&	image;
public:
	LuminanceAdapter(const ConstImageAdapter<Pixel>& image);
	const double	pixel(unsigned int x, unsigned int y) const;
};

template<typename Pixel>
LuminanceAdapter<Pixel>::LuminanceAdapter(
	const ConstImageAdapter<Pixel>& _image)
	: ConstImageAdapter<double>(_image.getSize()), image(_image) {
}

template<typename Pixel>
const double	LuminanceAdapter<Pixel>::pixel(unsigned int x, unsigned int y)
			const {
	return luminance(image.pixel(x, y));
}

//////////////////////////////////////////////////////////////////////
// Rescaling adapter
//////////////////////////////////////////////////////////////////////
template<typename Pixel>
class RescalingAdapter : public ConstImageAdapter<Pixel> {
	const ConstImageAdapter<Pixel>&	image;
	double	minpixel;
	double	scale;
	Pixel	zero;
public:
	RescalingAdapter(const ConstImageAdapter<Pixel>& image,
		double _minpixel, double scale);
	virtual const Pixel	pixel(unsigned int x, unsigned int y) const;
};

template<typename Pixel>
RescalingAdapter<Pixel>::RescalingAdapter(const ConstImageAdapter<Pixel>& _image,
		double _minpixel, double _scale)
		: ConstImageAdapter<Pixel>(_image.getSize()), image(_image),
		  minpixel(_minpixel), scale(_scale), zero(minpixel) {
}

template<typename Pixel>
const Pixel	RescalingAdapter<Pixel>::pixel(unsigned int x, unsigned int y)
	const {
	return (image.pixel(x, y) - zero) * scale;
}

} // namespace image
} // namespace astro
