/*
 * MeasureEvaluator.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroFocus.h>
#include "MeasureEvaluator.h"
#include <AstroFilter.h>
#include <AstroFilterfunc.h>

using namespace astro::adapter;
using namespace astro::image::filter;
using namespace astro::image;

namespace astro {
namespace focusing {

MeasureEvaluator::MeasureEvaluator() {
}

MeasureEvaluator::MeasureEvaluator(const ImageRectangle& roi)
	: FocusEvaluatorImplementation(roi) {
}

/**
 * \brief Evaluate an image based on a measure
 */
double	MeasureEvaluator::evaluate(FocusableImage image) {
	// compute the 
	FocusInfo       fi = astro::image::filter::focus_squaredgradient_extended(image);

	// find the maximum value of the edges
	Image<double>	*im = dynamic_cast<Image<double> *>(&*fi.edges);
	if (NULL == im) {
		throw std::runtime_error("edges image has incorrect pixel type");
	}
	double	maxvalue = Max<double, double>().filter(*im);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "maximum edge value: %f", maxvalue);
	

	// create an adapter that rescales to a reasonable value
	RescaleAdapter<double>	rescale(*im, maxvalue);
	TypeReductionAdapter<unsigned char, double>	tc(rescale);
	Image<unsigned char>	*red = new Image<unsigned char>(tc);
	ImagePtr	redptr(red);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "maximum red: %f",
		astro::image::filter::max(redptr));

	// Rescale the image to produce the green channel
	Image<unsigned char>	*green = UnsignedCharImage(image);
	ImagePtr	greenptr(green);

	// create a constant blue channel
	ConstantValueAdapter<unsigned char>	blue(image->size(), 0);

	// combine the channels to a color image
	CombinationAdapter<unsigned char>	combinator(*red, *green, blue);
	Image<RGB<unsigned char> >	*result
		= new Image<RGB<unsigned char> >(combinator);

	_evaluated_image = ImagePtr(result);

	// return the measure value
	return fi.value;
}

} // namespace focusing
} // namespace astro
