/*
 * MeasureEvaluator.h -- evaluator using the squaredgradient filter
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _MeasureEvaluator_h
#define _MeasureEvaluator_h

namespace astro {
namespace focusing {

class MeasureEvaluator : public FocusEvaluator {
public:
	MeasureEvaluator();
	MeasureEvaluator(const ImageRectangle& rectangle);
	virtual double	operator()(const ImagePtr image);
};

} // namespace focusing
} // namespace astro

#endif /* _MeasureEvaluator_h */
