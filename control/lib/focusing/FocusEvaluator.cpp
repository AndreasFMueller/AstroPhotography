/*
 * FocusEvaluator.cpp -- implementation of focus evaluators 
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTypes.h>
#include <AstroFocus.h>
#include <AstroDebug.h>

namespace astro {
namespace focusing {

/**
 * \brief Implementation base class for Focus evaluators
 */
class FocusEvaluatorImplementation : public FocusEvaluator {
protected:
	ImageRectangle	_rectangle;
	FocusableImage	extractimage(ImagePtr image);
public:
	FocusEvaluatorImplementation(const ImageRectangle& rectangle)
		: _rectangle(rectangle) { }
	virtual double	operator()(const ImagePtr image) = 0;
};

FocusableImage	FocusEvaluatorImplementation::extractimage(const ImagePtr image) {
	FocusableImageConverterPtr	converter
		= FocusableImageConverter::get();
	if (!converter) {
		throw std::runtime_error("cannot get an image converter");
	}
	return (*converter)(image);
}

/**
 * \brief Base class for Brenner type focus image adapters
 */
class BrennerAdapter : public ConstImageAdapter<float> {
protected:
	FocusableImage	_fim;
	int	_exponent;
	float	p(float x) const {
		return powf(fabsf(x), (float)_exponent);
	}
public:
	BrennerAdapter(FocusableImage fim, int exponent = 2)
		: ConstImageAdapter<float>(fim->size()), _fim(fim),
		  _exponent(exponent) {
	}
};

typedef std::shared_ptr<BrennerAdapter>	BrennerAdapterPtr;

/**
 * \brief Horizontal Brenner focus image adapter
 */
class BrennerHorizontalAdapter : public BrennerAdapter {
public:
	BrennerHorizontalAdapter(FocusableImage fim, int exponent = 2)
		: BrennerAdapter(fim, exponent) { }
	virtual float	pixel(int x, int y) const {
		if ((x > 0) && (x < getSize().width() - 1)) {
			return p(_fim->pixel(x+1, y) - _fim->pixel(x-1,y));
		}
		return 0;
	}
};

/**
 * \brief Vertical Brenner focus image adapter
 */
class BrennerVerticalAdapter : public BrennerAdapter {
public:
	BrennerVerticalAdapter(FocusableImage fim, int exponent)
		: BrennerAdapter(fim, exponent) { }
	virtual float	pixel(int x, int y) const {
		if ((y > 0) && (y < getSize().height() - 1)) {
			return p(_fim->pixel(x,y+1) - _fim->pixel(x,y-1));
		}
		return 0;
	}
};

/**
 * \brief Brenner omnidirectional adapter
 */
class BrennerOmniAdapter : public BrennerAdapter {
public:
	BrennerOmniAdapter(FocusableImage fim, int exponent)
		: BrennerAdapter(fim, exponent = 2) { }
	virtual float	pixel(int x, int y) const {
		if ((x > 0) && (x < getSize().width() - 1) 
			&& (y > 0) && (y < getSize().height() - 1)) {
			return p(_fim->pixel(x+1, y) - _fim->pixel(x-1,y))
				+ p(_fim->pixel(x,y+1) - _fim->pixel(x,y-1));
		}
		return 0;
	}
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
	BrennerEvaluatorBase(const ImageRectangle& rectangle, int exponent)
		: FocusEvaluatorImplementation(rectangle), _exponent(exponent) {
	}
	virtual double	operator()(const ImagePtr image);
}; 

double	BrennerEvaluatorBase::operator()(const ImagePtr image) {
	FocusableImage	fim = extractimage(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found image of size %s",
		fim->size().toString().c_str());
	BrennerAdapterPtr	a = this->adapter(fim, _exponent);
	if (!a) {
		throw std::runtime_error("cannot get an adapter");
	}
	double	sum = 0;
	for (int x = 1; x < fim->size().width() - 1; x++) {
		for (int y = 1; y < fim->size().height() - 1; y++) {
			sum += a->pixel(x, y);
		}
	}
	return sum;
}

/**
 * \brief Focus Evaluator for horizontal Brenner measure
 */
template<typename Adapter>
class BrennerEvaluator : public BrennerEvaluatorBase {
protected:
	virtual BrennerAdapterPtr	adapter(FocusableImage fim, int exponent);
public:
	BrennerEvaluator(const ImageRectangle& rectangle, int exponent = 2)
		: BrennerEvaluatorBase(rectangle, exponent) { }
};

template<typename Adapter>
BrennerAdapterPtr	BrennerEvaluator<Adapter>::adapter(FocusableImage fim, int exponent) {
	return BrennerAdapterPtr(new Adapter(fim, exponent));
}

typedef BrennerEvaluator<BrennerVerticalAdapter>	BrennerVerticalEvaluator;
typedef BrennerEvaluator<BrennerHorizontalAdapter>	BrennerHorizontalEvaluator;
typedef BrennerEvaluator<BrennerOmniAdapter>	BrennerOmniEvaluator;

//////////////////////////////////////////////////////////////////////
// FocusEvaluatorFactory Implementation
//////////////////////////////////////////////////////////////////////
FocusEvaluatorPtr	FocusEvaluatorFactory::get(FocusEvaluatorType type) {
	ImageRectangle	rectangle;
	return get(type, rectangle);
}

FocusEvaluatorPtr	FocusEvaluatorFactory::get(FocusEvaluatorType type,
				const ImageRectangle& rectangle) {
	FocusEvaluator	*evaluator = NULL;
	switch (type) {
	case BrennerHorizontal:
		evaluator = new BrennerHorizontalEvaluator(rectangle);
		break;
	case BrennerVertical:
		evaluator = new BrennerVerticalEvaluator(rectangle);
		break;
	case BrennerOmni:
		evaluator = new BrennerOmniEvaluator(rectangle);
		break;
	}
	if (NULL == evaluator) {
		debug(LOG_ERR, DEBUG_LOG, 0, "unknown evaluator type %d", type);
		throw std::runtime_error("unknown evaluator type");
	}
	return FocusEvaluatorPtr(evaluator);
}

} // namespace focusing
} // namespace astro

