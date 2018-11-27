/*
 * LRGBStep.cpp -- combine L and RGB into a single image
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <AstroDebug.h>
#include <AstroAdapter.h>

namespace astro {
namespace process {

ProcessingStep::state	LRGBStep::do_work() {
	ImageSequence	images = precursorimages();
	auto	i = images.begin();

	// get the luminance image
	ImagePtr	Lptr = *i++;
	Image<float>	*Limg
		= dynamic_cast<Image<float>*>(&*Lptr);
	if (NULL == Limg) {
		throw std::runtime_error("L image is not Image<float>");
	}

	// get the RGB image
	ImagePtr	RGBptr = *i;
	Image<RGB<float> >	*RGBimg
		= dynamic_cast<Image<RGB<float> >*>(&*RGBptr);
	if (NULL == RGBimg) {
		throw std::runtime_error("RGB image is not Image<RGB<float> >");
	}

	// make sure the sizes are consistent
	if (Limg->size() != RGBimg->size()) {
		std::string	msg = stringprintf("sizes inconsistent: "
			"%s != %s",
			Limg->size().toString().c_str(),
			RGBimg->size().toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// build the adapters
	adapter::ColorExtractionAdapter<float>	cea(*RGBimg);
	double	w = byid(*precursors().begin())->weight();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "saturation: %f", w);
	cea.saturation(w);
	adapter::LuminanceColorAdapter<float>	lca(*Limg, cea);

	// build the result image
	Image<RGB<float> >	*resultimg = new Image<RGB<float> >(lca);
	_image = ImagePtr(resultimg);

	return ProcessingStep::complete;
}

std::string	LRGBStep::what() const {
	return std::string("combine L and RGB");
}

} // namespace process
} // namespace astro
