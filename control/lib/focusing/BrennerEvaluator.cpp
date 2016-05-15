/*
 * BrennerEvaluator.cpp -- implementation of Brenner Focus evaluators
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTypes.h>
#include <AstroFocus.h>
#include <AstroDebug.h>
#include <AstroAdapter.h>
#include "BrennerEvaluator.h"

namespace astro {
namespace focusing {

//////////////////////////////////////////////////////////////////////
// BrennerAdapter implementation
//////////////////////////////////////////////////////////////////////
float	BrennerAdapter::p(float x) const {
	return powf(fabsf(x), (float)_exponent);
}

BrennerAdapter::BrennerAdapter(FocusableImage fim, int exponent)
	: ConstImageAdapter<float>(fim->size()), _fim(fim),
	  _exponent(exponent) {
}

typedef std::shared_ptr<BrennerAdapter>	BrennerAdapterPtr;

//////////////////////////////////////////////////////////////////////
// BrennerHorizontalAdapter implementation
//////////////////////////////////////////////////////////////////////
BrennerHorizontalAdapter::BrennerHorizontalAdapter(FocusableImage fim,
	int exponent) : BrennerAdapter(fim, exponent) {
}

float	BrennerHorizontalAdapter::pixel(int x, int y) const {
	if ((x > 0) && (x < getSize().width() - 1)) {
		return p(_fim->pixel(x+1, y) - _fim->pixel(x-1,y));
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////
// BrennerHorizontalAdapter implementation
//////////////////////////////////////////////////////////////////////
BrennerVerticalAdapter::BrennerVerticalAdapter(FocusableImage fim,
	int exponent) : BrennerAdapter(fim, exponent) {
}

float	BrennerVerticalAdapter::pixel(int x, int y) const {
	if ((y > 0) && (y < getSize().height() - 1)) {
		return p(_fim->pixel(x,y+1) - _fim->pixel(x,y-1));
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////
// BrennerOmniAdapter implementation
//////////////////////////////////////////////////////////////////////
BrennerOmniAdapter::BrennerOmniAdapter(FocusableImage fim, int exponent)
	: BrennerAdapter(fim, exponent) {
}

float	BrennerOmniAdapter::pixel(int x, int y) const {
	if ((x > 0) && (x < getSize().width() - 1) 
		&& (y > 0) && (y < getSize().height() - 1)) {
		return p(_fim->pixel(x+1, y) - _fim->pixel(x-1,y))
			+ p(_fim->pixel(x,y+1) - _fim->pixel(x,y-1));
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////
// BrennerEvaluatorBase class implementation
//////////////////////////////////////////////////////////////////////
BrennerEvaluatorBase::BrennerEvaluatorBase(const ImageRectangle& rectangle,
	int exponent) : FocusEvaluatorImplementation(rectangle),
			_exponent(exponent) {
}

double	BrennerEvaluatorBase::BrennerEvaluatorBase::operator()(
	const ImagePtr image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "evaluating an image of size %s",
		image->size().toString().c_str());
	FocusableImage	fim = extractimage(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found image of size %s",
		fim->size().toString().c_str());
	BrennerAdapterPtr	a = this->adapter(fim, _exponent);
	if (!a) {
		throw std::runtime_error("cannot get an adapter");
	}

	// compute the brenner measure, also find the maximum to be
	// used in rescaling the brenner image later
	double	max = 0;
	double	sum = 0;
	for (int x = 1; x < fim->size().width() - 1; x++) {
		for (int y = 1; y < fim->size().height() - 1; y++) {
			double	value = a->pixel(x, y);
			sum += value;
			if (value > max) {
				max = value;
			}
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "maximum value found: %f", max);

	// combine images into a loggable image
	Image<unsigned char>	*green = UnsignedCharImage(image);
	Image<unsigned char>	*red = new Image<unsigned char>(*a, 255. / max);
	adapter::CombinationAdapterPtr<unsigned char>	ca(red, green, NULL);
	_evaluated_image = ImagePtr(new Image<RGB<unsigned char> >(ca));
	
	// return the evaluated image
	return sum;
}

} // namespace focusing
} // namespace astro

