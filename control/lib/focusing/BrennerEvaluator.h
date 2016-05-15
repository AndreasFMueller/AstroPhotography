/*
 * BrennerEvaluator.h -- Brenner Evaluator definitions
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _BrennerEvaluator_h
#define _BrennerEvaluator_h

#include <AstroTypes.h>
#include <AstroFocus.h>
#include <AstroDebug.h>
#include "FocusEvaluator.h"

namespace astro {
namespace focusing {

/**
 * \brief Base class for Brenner type focus image adapters
 */
class BrennerAdapter : public ConstImageAdapter<float> {
protected:
	FocusableImage	_fim;
	int	_exponent;
	float	p(float x) const;
public:
	BrennerAdapter(FocusableImage fim, int exponent = 2);
};

typedef std::shared_ptr<BrennerAdapter>	BrennerAdapterPtr;

/**
 * \brief Horizontal Brenner focus image adapter
 */
class BrennerHorizontalAdapter : public BrennerAdapter {
public:
	BrennerHorizontalAdapter(FocusableImage fim, int exponent = 2);
	virtual float	pixel(int x, int y) const;
};

/**
 * \brief Vertical Brenner focus image adapter
 */
class BrennerVerticalAdapter : public BrennerAdapter {
public:
	BrennerVerticalAdapter(FocusableImage fim, int exponent = 2);
	virtual float	pixel(int x, int y) const;
};

/**
 * \brief Brenner omnidirectional adapter
 */
class BrennerOmniAdapter : public BrennerAdapter {
public:
	BrennerOmniAdapter(FocusableImage fim, int exponent = 2);
	virtual float	pixel(int x, int y) const;
};

/**
 * \brief Brenner Focus evaluator base class
 *
 * These evaluators use Brenner adapters to extract the image and sum the
 * brenner focus values. The sum over the pixel values is done in the base
 * class.
 */
class BrennerEvaluatorBase : public FocusEvaluatorImplementation {
protected:
	virtual BrennerAdapterPtr	adapter(FocusableImage fim,
						int exponent) = 0;
private:
	int	_exponent;
public:
	BrennerEvaluatorBase(const ImageRectangle& rectangle, int exponent);
	virtual double	operator()(const ImagePtr image);
}; 

/**
 * \brief Focus Evaluator for horizontal Brenner measure
 */
template<typename Adapter>
class BrennerEvaluator : public BrennerEvaluatorBase {
protected:
	virtual BrennerAdapterPtr	adapter(FocusableImage fim,
		int exponent);
public:
	BrennerEvaluator(const ImageRectangle& rectangle, int exponent = 2)
		: BrennerEvaluatorBase(rectangle, exponent) { }
};

template<typename Adapter>
BrennerAdapterPtr	BrennerEvaluator<Adapter>::adapter(FocusableImage fim,
				int exponent) {
	return BrennerAdapterPtr(new Adapter(fim, exponent));
}

typedef BrennerEvaluator<BrennerVerticalAdapter>	BrennerVerticalEvaluator;
typedef BrennerEvaluator<BrennerHorizontalAdapter>	BrennerHorizontalEvaluator;
typedef BrennerEvaluator<BrennerOmniAdapter>	BrennerOmniEvaluator;

} // namespace focusing
} // namespace astro

#endif /* _BrennerEvaluator_h */
