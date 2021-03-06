/*
 * AstroTonemapping.h -- Tonemapping adapters for floating point images
 */
#ifndef _AstroTonemapping_h
#define _AstroTonemapping_h

#include <AstroAdapter.h>
#include <cmath>

namespace astro {
namespace adapter {

/**
 * \brief Gamma Adapter
 */
template<typename Pixel>
class GammaAdapter : public ConstImageAdapter<Pixel> {
	const ConstImageAdapter<Pixel>&	image;
	float	_gamma;
	float	_min;
	float	_max;
public:
	GammaAdapter(const ConstImageAdapter<Pixel>& image, const float gamma = 1);
	virtual Pixel	pixel(int x, int y) const;
	float	gamma() const { return _gamma; }
	void	gamma(float gamma) { _gamma = gamma; }
	void	min(float m) { _min = m; }
	void	max(float m) { _max = m; }
};

template<typename Pixel>
GammaAdapter<Pixel>::GammaAdapter(const ConstImageAdapter<Pixel>& _image,
	const float gamma)
	: ConstImageAdapter<Pixel>(_image.getSize()), image(_image) {
	_gamma = gamma;
	_min = 0.;
	_max = 1.;
}

template<typename Pixel>
Pixel	GammaAdapter<Pixel>::pixel(int x, int y) const {
	Pixel	p = image.pixel(x, y);
	double	v = p;
	if (v < _min) {
		return Pixel(0);
	}
	double	s = _max - _min;
	return p * s *  pow((v - _min) / s, _gamma);
}

/**
 * \brief Cauchy Adapter
 */
template<typename Pixel>
class CauchyAdapter : public ConstImageAdapter<Pixel> {
	const ConstImageAdapter<Pixel>&	image;
public:
	CauchyAdapter(const ConstImageAdapter<Pixel>& image);
	virtual Pixel	pixel(int x, int y) const;
};

template<typename Pixel>
CauchyAdapter<Pixel>::CauchyAdapter(const ConstImageAdapter<Pixel>& _image)
	: ConstImageAdapter<Pixel>(_image.getSize()), image(_image) {
}

template<typename Pixel>
Pixel	CauchyAdapter<Pixel>::pixel(int x, int y)
	const {
	double	l = image.pixel(x, y);
	return l / (l + 1.);
}

/**
 * \brief Rescaling Adapter
 */
template<typename Pixel>
class LuminanceScalingAdapter : public ConstImageAdapter<Pixel> {
	const ConstImageAdapter<Pixel>&	image;
	double	_scalefactor;
public:
	LuminanceScalingAdapter(const ConstImageAdapter<Pixel>& image,
		double scalefactor = 1);
	virtual Pixel	pixel(int x, int y) const;
	double	scalefactor() const { return _scalefactor; }
	void	scalefactor(double scalefactor) { _scalefactor = scalefactor; }
};

template<typename Pixel>
LuminanceScalingAdapter<Pixel>::LuminanceScalingAdapter(
	const ConstImageAdapter<Pixel>& _image, double scalefactor)
	: ConstImageAdapter<Pixel>(_image.getSize()), image(_image),
	  _scalefactor(scalefactor) {
}

template<typename Pixel>
Pixel	LuminanceScalingAdapter<Pixel>::pixel(int x, int y) const {
	return image.pixel(x, y) * _scalefactor;
}

/**
 * \brief Adapter for an arbitrary pixel function
 */
template<typename Pixel, typename PixelFunction>
class PixelFunctionAdapter : public ConstImageAdapter<Pixel> {
	const ConstImageAdapter<Pixel>&	_image;
	PixelFunction	_pixelfunction;
public:
	PixelFunctionAdapter(const ConstImageAdapter<Pixel>& image,
		const PixelFunction& pixelfunction);
	virtual Pixel	pixel(int x, int y) const;
};

template<typename Pixel, typename PixelFunction>
PixelFunctionAdapter<Pixel, PixelFunction>::PixelFunctionAdapter(
	const ConstImageAdapter<Pixel>& image,
	const PixelFunction& pixelfunction)
	: ConstImageAdapter<Pixel>(image.getSize()),
	  _image(image), _pixelfunction(pixelfunction) {
}
	
template<typename Pixel, typename PixelFunction>
Pixel	PixelFunctionAdapter<Pixel, PixelFunction>::pixel(int x, int y) const {
	Pixel	result = _pixelfunction(_image.pixel(x, y));
}

/**
 * \brief Adapter for a pixel function that affects only the luminance
 */
template<typename Pixel, typename LuminanceFunction>
class LuminanceFunctionAdapter : public ConstImageAdapter<Pixel> {
	const ConstImageAdapter<Pixel>&	_image;
	LuminanceFunction	_luminancefunction;
public:
	LuminanceFunctionAdapter(const ConstImageAdapter<Pixel>& image,
		const LuminanceFunction& luminancefunction);
	virtual Pixel	pixel(int x, int y) const;
};

template<typename Pixel, typename LuminanceFunction>
LuminanceFunctionAdapter<Pixel, LuminanceFunction>::LuminanceFunctionAdapter(
	const ConstImageAdapter<Pixel>& image,
	const LuminanceFunction& luminancefunction)
	: ConstImageAdapter<Pixel>(image.getSize()),
	  _image(image), _luminancefunction(luminancefunction) {
}
	
template<typename Pixel, typename LuminanceFunction>
Pixel	LuminanceFunctionAdapter<Pixel, LuminanceFunction>::pixel(int x, int y)
	const {
	Pixel	p = _image.pixel(x, y);
	double	l = p;
	_luminancefunction(l);
	Pixel	result = p * l;
	return result;
}

/**
 * \brief Base class for Luminance Functions
 */
class LuminanceFunction	{
	std::string	_name;
	double	_x1;
	double	_y1;
	double	_x2;
	double	_y2;
	bool	_use_absolute;
	bool	_truncate_negative;
public:
	const std::string& 	name() const { return _name; }
	double	x1() const { return _x1; }
	double	x2() const { return _x2; }
	void	x1(double l) { _x1 = l; }
	void	x2(double l) { _x2 = l; }
	double	y1() const { return _y1; }
	double	y2() const { return _y2; }
	void	y1(double l) { _y1 = l; }
	void	y2(double l) { _y2 = l; }
	bool	use_absolute() const { return _use_absolute; }
	void	use_absolute(bool u) { _use_absolute = u; }
	bool	truncate_negative() const { return _truncate_negative; }
	void	truncate_negative(bool t) { _truncate_negative = t; }
protected:
	double	x(double l) const;
	double	y(double x) const;
public:
	typedef std::map<std::string, std::string>	parameters_t;
	LuminanceFunction(const std::string& name);
	LuminanceFunction(const std::string& name,
		const parameters_t& parameters);
	virtual ~LuminanceFunction();
	virtual double	operator()(double l) = 0;
	virtual std::string	info() const;
};

typedef std::shared_ptr<LuminanceFunction>	LuminanceFunctionPtr;

/**
 * \brief A variant of the LuminanceFunctionAdapter class for smart pointers
 */
template<typename Pixel>
class LuminanceFunctionPtrAdapter : public ConstImageAdapter<Pixel> {
	const ConstImageAdapter<Pixel>&	_image;
	LuminanceFunctionPtr		_luminancefunctionptr;
public:
	LuminanceFunctionPtrAdapter(const ConstImageAdapter<Pixel>& image,
		LuminanceFunctionPtr luminancefunctionptr)
		: ConstImageAdapter<Pixel>(image.getSize()), _image(image),
		  _luminancefunctionptr(luminancefunctionptr) {
	}
	virtual Pixel	pixel(int x, int y) const {
		Pixel	p = _image.pixel(x, y);
		double	l = p;
		return p * _luminancefunctionptr->operator()(l);
	}
};

ImagePtr	luminancemapping(ImagePtr image,
			LuminanceFunctionPtr luminancefunctionptr);

/**
 * \brief A factory for LuminanceFunctions
 */
class LuminanceFunctionFactory {
public:
	static LuminanceFunctionPtr	get(const std::string& name);
	static LuminanceFunctionPtr	get(const std::string& name,
		const LuminanceFunction::parameters_t& parameters);
};

/**
 * \brief Luminance extraction
 */
template<typename Pixel>
class LuminanceExtractionAdapter : public ConstImageAdapter<Pixel> {
	const ConstImageAdapter<RGB<Pixel> >&	_image;
public:
	LuminanceExtractionAdapter(const ConstImageAdapter<RGB<Pixel> >& image);
	virtual Pixel	pixel(int x, int y) const;
};

template<typename Pixel>
LuminanceExtractionAdapter<Pixel>::LuminanceExtractionAdapter(
	const ConstImageAdapter<RGB<Pixel> >& image)
	: ConstImageAdapter<Pixel>(image.getSize()), _image(image) {
}

template<typename Pixel>
Pixel	LuminanceExtractionAdapter<Pixel>::pixel(int x, int y) const {
	return _image.pixel(x, y).luminance();
}

/**
 * \brief Color extraction
 */
template<typename Pixel>
class ColorExtractionAdapter : public ConstImageAdapter<RGB<Pixel> > {
	const ConstImageAdapter<RGB<Pixel> >&	_image;
	double	_saturation;
public:
	ColorExtractionAdapter(const ConstImageAdapter<RGB<Pixel> >& image);
	virtual RGB<Pixel> pixel(int x, int y) const;
	double	saturation() const { return _saturation; }
	void	saturation(double saturation) { _saturation = saturation; }
};

template<typename Pixel>
ColorExtractionAdapter<Pixel>::ColorExtractionAdapter(
	const ConstImageAdapter<RGB<Pixel> >& image)
	: ConstImageAdapter<RGB<Pixel> >(image.getSize()), _image(image) {
	_saturation = 1.;
}

template<typename Pixel>
RGB<Pixel>	ColorExtractionAdapter<Pixel>::pixel(int x, int y) const {
	RGB<Pixel>	v = _image.pixel(x, y);
	float	l = v.luminance();
	RGB<Pixel>	c = v * l;
	Pixel	R = 1 + (v.R / l - 1) * _saturation;
	Pixel	G = 1 + (v.G / l - 1) * _saturation;
	Pixel	B = 1 + (v.B / l - 1) * _saturation;
	return RGB<Pixel>(R, G, B);
}

/**
 * \brief Luminance-Color-Combination Adapter
 */
template<typename Pixel>
class LuminanceColorAdapter : public ConstImageAdapter<RGB<Pixel> > {
	const ConstImageAdapter<Pixel>&		luminanceimage;
	const ConstImageAdapter<RGB<Pixel> >&	colorimage;
public:
	LuminanceColorAdapter(
		const ConstImageAdapter<Pixel>& luminanceimage,
		const ConstImageAdapter<RGB<Pixel> >& colorimage);
	virtual RGB<Pixel>	pixel(int x, int y) const;
};

template<typename Pixel>
LuminanceColorAdapter<Pixel>::LuminanceColorAdapter(
	const ConstImageAdapter<Pixel>& _luminanceimage,
	const ConstImageAdapter<RGB<Pixel> >& _colorimage)
	: ConstImageAdapter<RGB<Pixel> >(_luminanceimage.getSize()),
	  luminanceimage(_luminanceimage), colorimage(_colorimage) {
}

template<typename Pixel>
RGB<Pixel>	LuminanceColorAdapter<Pixel>::pixel(int x, int y) const {
	return colorimage.pixel(x, y) * luminanceimage.pixel(x, y);
}

/**
 * \brief ColorCorrection Adapter
 */
template<typename Pixel>
class ColorCorrectionAdapter : public ConstImageAdapter<RGB<Pixel> > {
	const ConstImageAdapter<RGB<Pixel> >&	image;
	RGB<float>	_rgb;
public:
	ColorCorrectionAdapter<Pixel>(
		const ConstImageAdapter<RGB<Pixel> >& image);
	ColorCorrectionAdapter<Pixel>(
		const ConstImageAdapter<RGB<Pixel> >& image,
		const RGB<float>& rgb);
	virtual RGB<Pixel>	pixel(int x, int y) const;
	RGB<float>	rgb() const { return _rgb; }
	void	rgb(const RGB<float>& rgb) { _rgb = rgb; }
};

template<typename Pixel>
ColorCorrectionAdapter<Pixel>::ColorCorrectionAdapter(
	const ConstImageAdapter<RGB<Pixel> >& _image)
	: ConstImageAdapter<RGB<Pixel> >(_image.getSize()),
	  image(_image), _rgb(1., 1., 1.) {
	_rgb = _rgb / _rgb.luminance();
}

template<typename Pixel>
ColorCorrectionAdapter<Pixel>::ColorCorrectionAdapter(
	const ConstImageAdapter<RGB<Pixel> >& _image, const RGB<float>& rgb)
	: ConstImageAdapter<RGB<Pixel> >(_image.getSize()), image(_image) {
	_rgb = rgb / rgb.luminance();
}

template<typename Pixel>
RGB<Pixel>	ColorCorrectionAdapter<Pixel>::pixel(int x, int y) const {
	RGB<Pixel>	v = image.pixel(x, y);
	return RGB<Pixel>(v.R * _rgb.R, v.G * _rgb.G, v.B * _rgb.B);
}

/**
 * \brief Backgroup subtraction adapter
 */
template<typename Pixel>
class BackgroundAdapter : public ConstImageAdapter<RGB<Pixel> > {
	const ConstImageAdapter<RGB<Pixel> >&	image;
	RGB<Pixel>	background;
public:
	BackgroundAdapter(
		const ConstImageAdapter<RGB<Pixel> >& image,
		const RGB<Pixel>& background);
	virtual RGB<Pixel>	pixel(int x, int y) const;
};

template<typename Pixel>
BackgroundAdapter<Pixel>::BackgroundAdapter(
	const ConstImageAdapter<RGB<Pixel> >& _image,
	const RGB<Pixel>& _background)
	: ConstImageAdapter<RGB<Pixel> >(_image.getSize()), image(_image),
	  background(_background) {
}

template<typename Pixel>
RGB<Pixel>	BackgroundAdapter<Pixel>::pixel(int x, int y) const {
	RGB<Pixel>	v = image.pixel(x, y);
	return RGB<Pixel>(v.R - background.R, v.G - background.G,
		v.B - background.B);
}

/**
 * \brief Range adapter
 */
template<typename Pixel>
class RangeAdapter : public ConstImageAdapter<Pixel> {
	const ConstImageAdapter<Pixel>&	_image;
	float	m;
	float	b;
public:
	RangeAdapter(const ConstImageAdapter<Pixel>& image,
		float min = 0, float max = 1);
	virtual Pixel	pixel(int x, int y) const;
	double	min() const { return -b; }
	double	max() const { return 1/m - b; }
	void	setRange(float min = 0, float max = 1);
};

template<typename Pixel>
RangeAdapter<Pixel>::RangeAdapter(const ConstImageAdapter<Pixel>& image,
	float min, float max)
	: ConstImageAdapter<Pixel>(image.getSize()), _image(image) {
	setRange(min, max);
}

template<typename Pixel>
void	RangeAdapter<Pixel>::setRange(float min, float max) {
	b = -min;
	m = 1. / (max - min);
}

template<typename Pixel>
Pixel RangeAdapter<Pixel>::pixel(int x, int y) const {
	return m * (_image.pixel(x, y) + b);
}

/**
 * \brief RGB32 extraction
 */
template<typename Pixel>
class RGB32Adapter : public ConstImageAdapter<unsigned int> {
	const ConstImageAdapter<RGB<Pixel> >&	_image;
	unsigned char	reduce(Pixel v) const {
		if (v > 255) {
			v = 255;
		}
		if (v < 0) {
			v = 0;
		}
		unsigned char	result = v;
		return result;
	}
	unsigned int	reduce(const RGB<Pixel>& v) const {
		unsigned int	result = reduce(v.R);
		result <<= 8;
		result |= reduce(v.G);
		result <<= 8;
		result |= reduce(v.B);
		return result;
	}
public:
	RGB32Adapter(const ConstImageAdapter<RGB<Pixel> >& image);
	virtual unsigned int	pixel(int x, int y) const;
};

template<typename Pixel>
RGB32Adapter<Pixel>::RGB32Adapter(const ConstImageAdapter<RGB<Pixel> >& image)
	: ConstImageAdapter<unsigned int>(image.getSize()), _image(image) {
}

template<typename Pixel>
unsigned int	RGB32Adapter<Pixel>::pixel(int x, int y) const {
	return reduce(_image.pixel(x, y));
}

/**
 * \brief Default Luminance Factor
 *
 * This factor always returns 1 so it does not change the image
 */
class LuminanceFactor {
public:
	virtual double	operator()(double /* d */) { return 1; }
};
typedef std::shared_ptr<LuminanceFactor>	LuminanceFactorPtr;

/**
 * \brief Class to apply an arbitrary luminance mapping
 */
template<typename Pixel>
class LuminanceStretchingAdapter : public ConstImageAdapter<Pixel> {
	const ConstImageAdapter<Pixel>&	_image;
	LuminanceAdapter<Pixel, double>	_luminance;
	LuminanceFactor&	_factor;
public:
	LuminanceStretchingAdapter(const ConstImageAdapter<Pixel>& image,
		LuminanceFactor& factor)
		: ConstImageAdapter<Pixel>(image.getSize()), _image(image),
		  _luminance(image), _factor(factor) {
	}
	virtual Pixel	pixel(int x, int y) const {
		Pixel	v = _image.pixel(x, y);
		double	l = _luminance.pixel(x, y);
		if (l < 0) {
			return Pixel(0);
		}
		return v * _factor(l);
	}
};

class LinearLogLuminanceFactor : public LuminanceFactor {
	double	_crossover;
	double	_top;
	double	_maximum;
	double	_s;
public:
	LinearLogLuminanceFactor(double crossover, double top, double maximum);
	virtual ~LinearLogLuminanceFactor() { }
	virtual double	operator()(double d);
};

template<typename Pixel>
Image<Pixel>	*luminancestretching(const Image<Pixel>& image,
			LuminanceFactor& factor) {
	LuminanceStretchingAdapter<Pixel>	lsa(image, factor);
	return new Image<Pixel>(lsa);
}

ImagePtr	luminancestretching(ImagePtr image, LuminanceFactor& factor);

} // namespace adapter
} // namespace astro

#endif /* _AstroTonemapping_h */
