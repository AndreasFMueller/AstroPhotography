/*
 * FocusEvaluator.cpp -- implementation of focus evaluators 
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTypes.h>
#include <AstroFocus.h>

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
	return (*converter)(image);
}

static float sqr(float x) {
	return x * x;
}

/**
 * \brief Base class for Brenner type focus image adapters
 */
class BrennerAdapter : public ConstImageAdapter<float> {
protected:
	FocusableImage	_fim;
public:
	BrennerAdapter(FocusableImage fim)
		: ConstImageAdapter<float>(fim->size()), _fim(fim) {
	}
};

typedef std::shared_ptr<BrennerAdapter>	BrennerAdapterPtr;

/**
 * \brief Horizontal Brenner focus image adapter
 */
class BrennerHorizontalAdapter : public BrennerAdapter {
	FocusableImage	_fim;
public:
	BrennerHorizontalAdapter(FocusableImage _fim) : BrennerAdapter(_fim) { }
	virtual float	pixel(int x, int y) const {
		if ((x > 0) && (x < getSize().width() - 1)) {
			return sqr(_fim->pixel(x+1, y) - _fim->pixel(x-1,y));
		}
		return 0;
	}
};

/**
 * \brief Vertical Brenner focus image adapter
 */
class BrennerVerticalAdapter : public BrennerAdapter {
	FocusableImage	_fim;
public:
	BrennerVerticalAdapter(FocusableImage _fim) : BrennerAdapter(_fim) { }
	virtual float	pixel(int x, int y) const {
		if ((y > 0) && (y < getSize().height() - 1)) {
			return sqr(_fim->pixel(x,y+1) - _fim->pixel(x,y-1));
		}
		return 0;
	}
};

/**
 * \brief Brenner Focus evaluators
 *
 * These evaluators use Brenner adapters to extract the image and sum the
 * brenner focus values.
 */
class BrennerEvaluator : public FocusEvaluatorImplementation {
protected:
	virtual BrennerAdapterPtr	adapter(FocusableImage fim) = 0;
public:
	BrennerEvaluator(const ImageRectangle& rectangle)
		: FocusEvaluatorImplementation(rectangle) {
	}
	virtual double	operator()(const ImagePtr image);
}; 

double	BrennerEvaluator::operator()(const ImagePtr image) {
	FocusableImage	fim = extractimage(image);
	BrennerAdapterPtr	a = adapter(fim);
	double	sum = 0;
	for (int x = 0; x < image->size().width() - 1; x++) {
		for (int y = 0; y < image->size().width() - 1; y++) {
			sum += a->pixel(x, y);
		}
	}
	return sum;
}

class BrennerHorizontalEvaluator : public BrennerEvaluator {
protected:
	virtual BrennerAdapterPtr	adapter(FocusableImage fim);
public:
	BrennerHorizontalEvaluator(const ImageRectangle& rectangle)
		: BrennerEvaluator(rectangle) { }
};

BrennerAdapterPtr	BrennerHorizontalAdapter::adapter(FocusableImage fim) {
	return BrennerAdapterPtr(new BrennerHorizontalAdapter(fim));
}

class BrennerVerticalEvaluator : public BrennerEvaluator {
protected:
	virtual BrennerAdapterPtr	adapter(FocusableImage fim);
public:
	BrennerVerticalEvaluator(const ImageRectangle& rectangle)
		: BrennerEvaluator(rectangle) { }
};

BrennerAdapterPtr	BrennerVerticalAdapter::adapter(FocusableImage fim) {
	return BrennerAdapterPtr(new BrennerVerticalAdapter(fim));
}


} // namespace focusing
} // namespace astro
