/*
 * FocusEvaluator.h -- implementation of focus evaluators 
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _FocusEvaluatorImplementation_h
#define _FocusEvaluatorImplementation_h

#include <AstroTypes.h>
#include <AstroFocus.h>
#include <AstroDebug.h>

namespace astro {
namespace focusing {

/**
 * \brief Implementation base class for Focus evaluators
 */
class FocusEvaluatorImplementation : public FocusEvaluator {
	ImageRectangle	_rectangle;
	FocusableImage	extractimage(ImagePtr image);
protected:
	virtual double	evaluate(FocusableImage image) = 0;
public:
	FocusEvaluatorImplementation();
	FocusEvaluatorImplementation(const ImageRectangle& rectangle);
	virtual double	operator()(const ImagePtr image) final;
};

} // namespace focusing
} // namespace astro

#endif /* _FocusEvaluatorImplementation_h */
