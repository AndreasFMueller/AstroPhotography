/*
 * AstroTonemapping.h -- Tonemapping adapters for floating point images
 */
#ifndef _AstroTonemapping_h
#define _AstroTonemapping_h

#include <AstroAdapter.h>
#include <math.h>

namespace astro {
namespace adapter {

/**
 * \brief Gamma Adapter
 */
template<typename Pixel>
class GammaAdapter : public ConstImageAdapter<Pixel> {
	const ConstImageAdapter<Pixel>&	image;
	float	_gamma;
public:
	GammaAdapter(const ConstImageAdapter<Pixel>& image, const float gamma);
	virtual const	Pixel	pixel(unsigned int x, unsigned int y) const;
};

template<typename Pixel>
GammaAdapter<Pixel>::GammaAdapter(const ConstImageAdapter<Pixel>& _image,
	const float gamma)
	: ConstImageAdapter<Pixel>(_image.getSize()), image(_image) {
	_gamma = gamma;
}

template<typename Pixel>
const Pixel	GammaAdapter<Pixel>::pixel(unsigned int x, unsigned int y) const {
	return pow(image.pixel(x, y), _gamma);
}

/**
 * \brief Cauchy Adapter
 */
template<typename Pixel>
class CauchyAdapter : public ConstImageAdapter<Pixel> {
	const ConstImageAdapter<Pixel>&	image;
public:
	CauchyAdapter(const ConstImageAdapter<Pixel>& image);
	virtual const Pixel	pixel(unsigned int x, unsigned int y) const;
};

template<typename Pixel>
CauchyAdapter<Pixel>::CauchyAdapter(const ConstImageAdapter<Pixel>& _image)
	: ConstImageAdapter<Pixel>(_image.getSize()), image(_image) {
}

template<typename Pixel>
const Pixel	CauchyAdapter<Pixel>::pixel(unsigned int x, unsigned int y)
	const {
	double	l = image.pixel(x, y);
	return l / (l + 1.);
}

/**
 * \brief LogAdapter
 */
template<typename Pixel>
class LogAdapter : public ConstImageAdapter<Pixel> {
	const ConstImageAdapter<Pixel>&	image;
public:
	LogAdapter(const ConstImageAdapter<Pixel>& image);
	virtual const Pixel	pixel(unsigned int x, unsigned int y) const;
};

template<typename Pixel>
LogAdapter<Pixel>::LogAdapter(const ConstImageAdapter<Pixel>& _image)
	: ConstImageAdapter<Pixel>(_image.getSize()), image(_image) {
}

template<typename Pixel>
const Pixel	LogAdapter<Pixel>::pixel(unsigned int x, unsigned int y) const {
	return log10(image.pixel(x, y));
}

/**
 * \brief Rescaling Adapter
 */
template<typename Pixel>
class LuminanceScalingAdapter : public ConstImageAdapter<Pixel> {
	const ConstImageAdapter<Pixel>&	image;
	double	scalefactor;
public:
	LuminanceScalingAdapter(const ConstImageAdapter<Pixel>& image,
		double scalefactor);
	virtual const Pixel	pixel(unsigned int x, unsigned int y) const;
};

template<typename Pixel>
LuminanceScalingAdapter<Pixel>::LuminanceScalingAdapter(
	const ConstImageAdapter<Pixel>& _image, double _scalefactor)
	: ConstImageAdapter<Pixel>(_image.getSize()), image(_image),
	  scalefactor(_scalefactor) {
}

template<typename Pixel>
const Pixel	LuminanceScalingAdapter<Pixel>::pixel(unsigned int x, unsigned int y) const {
	return image.pixel(x, y) * scalefactor;
}

/**
 * \brief Luminance-Color-Combination Adapter
 */
template<typename Pixel>
class LuminanceColorAdapter : public ConstImageAdapter<RGB<Pixel> > {
	const ConstImageAdapter<Pixel>&		luminanceimage;
	const ConstImageAdapter<RGB<Pixel> >&	colorimage;
public:
	LuminanceColorAdapter(const ConstImageAdapter<Pixel>& luminanceimage,
		const ConstImageAdapter<RGB<Pixel> >& colorimage);
	virtual const RGB<Pixel>	pixel(unsigned int x, unsigned int y) const;
};

template<typename Pixel>
LuminanceColorAdapter<Pixel>::LuminanceColorAdapter(
	const ConstImageAdapter<Pixel>& _luminanceimage,
	const ConstImageAdapter<RGB<Pixel> >& _colorimage)
	: ConstImageAdapter<RGB<Pixel> >(_luminanceimage.getSize()),
	  luminanceimage(_luminanceimage), colorimage(_colorimage) {
}

template<typename Pixel>
const RGB<Pixel>	LuminanceColorAdapter<Pixel>::pixel(
				unsigned int x, unsigned int y) const {
	return colorimage.pixel(x, y) * luminanceimage.pixel(x, y);
}

/**
 * \brief ColorCorrection Adapter
 */
template<typename Pixel>
class ColorCorrectionAdapter : public ConstImageAdapter<RGB<Pixel> > {
	const ConstImageAdapter<RGB<Pixel> >&	image;
	RGB<float>	rgb;
public:
	ColorCorrectionAdapter<Pixel>(const ConstImageAdapter<RGB<Pixel> >& image, const RGB<float>& rgb);
	virtual const RGB<Pixel>	pixel(unsigned int x, unsigned int y) const;
};

template<typename Pixel>
ColorCorrectionAdapter<Pixel>::ColorCorrectionAdapter(
	const ConstImageAdapter<RGB<Pixel> >& _image, const RGB<float>& _rgb)
	: ConstImageAdapter<RGB<Pixel> >(_image.getSize()), image(_image) {
	rgb = _rgb / _rgb.luminance();
}

template<typename Pixel>
const RGB<Pixel>	ColorCorrectionAdapter<Pixel>::pixel(unsigned int x, unsigned int y) const {
	RGB<Pixel>	v = image.pixel(x, y);
	return RGB<Pixel>(v.R * rgb.R, v.G * rgb.G, v.B * rgb.B);
}

/**
 * \brief Backgroup subtraction adapter
 */
template<typename Pixel>
class BackgroundSubtractionAdapter : public ConstImageAdapter<RGB<Pixel> > {
	const ConstImageAdapter<RGB<Pixel> >&	image;
	RGB<Pixel>	background;
public:
	BackgroundSubtractionAdapter(
		const ConstImageAdapter<RGB<Pixel> >& image,
		const RGB<Pixel>& background);
	virtual const RGB<Pixel>	pixel(unsigned int x, unsigned int y)
						const;
};

template<typename Pixel>
BackgroundSubtractionAdapter<Pixel>::BackgroundSubtractionAdapter(
	const ConstImageAdapter<RGB<Pixel> >& _image,
	const RGB<Pixel>& _background)
	: ConstImageAdapter<RGB<Pixel> >(_image.getSize()), image(_image),
	  background(_background) {
}

template<typename Pixel>
const RGB<Pixel>	BackgroundSubtractionAdapter<Pixel>::pixel(
	unsigned int x, unsigned int y) const {
	RGB<Pixel>	v = image.pixel(x, y);
	return RGB<Pixel>(v.R - background.R, v.G - background.G,
		v.B - background.B);
}

} // namespace adapter
} // namespace astro

#endif /* _AstroTonemapping_h */
