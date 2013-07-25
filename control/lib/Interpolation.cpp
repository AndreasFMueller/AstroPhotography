/**
 * Interpolation.cpp -- interpolate images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroInterpolation.h>
#include <PixelValue.h>
#include <limits>
#include <AstroDebug.h>
#include <stdexcept>

using namespace astro::image;

namespace astro {
namespace interpolation {

//////////////////////////////////////////////////////////////////////
// TypedInterpolator implementation
//////////////////////////////////////////////////////////////////////
template<typename T>
class TypedInterpolator {
	const Image<T>&	dark;
protected:
	virtual void	interpolatePixel(unsigned int x, unsigned int y,
				ImagePtr& image);
	PixelValue<T>	*pv;
	T	nan;
public:
	TypedInterpolator(const Image<T>& _dark);
	void	interpolate(ImagePtr& image);
};

template<typename T>
void	TypedInterpolator<T>::interpolatePixel(unsigned int x, unsigned int y,
		ImagePtr& image) {
	// XXX interpolation missing
}

template<typename T>
TypedInterpolator<T>::TypedInterpolator(const Image<T>& _dark) : dark(_dark) {
	nan = std::numeric_limits<T>::quiet_NaN();
}

template<typename T>
void	TypedInterpolator<T>::interpolate(ImagePtr& image) {
	// make sure the image sizes match
	if (image->size != dark.size) {
		throw std::range_error("image sizes don't match");
	}
	pv = new PixelValue<T>(image);
	for (unsigned int x = 0; x < dark.size.width; x++) {
		for (unsigned int y = 0; y < dark.size.height; y++) {
			if (dark.pixel(x, y) != dark.pixel(x, y)) {
				interpolatePixel(x, y, image);
			}
		}
	}
	delete pv;
}

//////////////////////////////////////////////////////////////////////
// Interpolator implementation
//////////////////////////////////////////////////////////////////////
Interpolator::Interpolator(const ImagePtr& _dark) : dark(_dark) {
	floatdark = dynamic_cast<Image<float> *>(&*dark);
	doubledark = dynamic_cast<Image<double> *>(&*dark);
	if ((NULL == floatdark) && (NULL == doubledark)) {
		throw std::runtime_error("only float or double images are "
			"suitable as darks");
	}
}

void	Interpolator::interpolate(ImagePtr& image) {
	if (floatdark) {
		TypedInterpolator<float>	tint(*floatdark);
		tint.interpolate(image);
		return;
	}
	if (doubledark) {
		TypedInterpolator<double>	tint(*doubledark);
		tint.interpolate(image);
		return;
	}
}

ImagePtr	Interpolator::operator()(const ImagePtr& image) {
	ImagePtr	imagecopy;
	interpolate(imagecopy);
	return imagecopy;
}

} // interpolation
} // astro
