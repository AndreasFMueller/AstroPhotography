/*
 * FocusEvaluator.cpp -- implementation of focus evaluators 
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTypes.h>
#include <AstroFocusing.h>

namespace astro {
namespace focusing {

/**
 * \brief Implementation base class for Focus evaluators
 */
class FocusEvaluatorImplementation : public FocusEvaluator {
protected:
	ImageRectangle	_rectangle;
	FocusableImage	extractimage(const ImagePtr image);
public:
	FocusEvaluatorImplementation(const ImageRectangle& rectangle)
		: _rectangle(rectangle) { }
	virtual double	operator()(const ImagePtr image) = 0;
}

FocusableImage	FocusEvaluatorImplementation::extractimage(const ImagePtr imge) {
	FocusableImageConverterPtr	converter
		= FocusableImageConverter::get();
	return (*converter)(image);
}

class BrennerHorizontalEvaluator : public FocusEvaluatorImplementation {
public:
	BrennerHorizontalEvaluator(const ImageRectangle& rectangle)
		: FocusEvaluatorImplementation(rectangle) { }
	virtual double	operator()(const ImagePtr image);
};


} // namespace focusing
} // namespace astro
