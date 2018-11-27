/*
 * SumStep.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>

namespace astro {
namespace process {

template<typename Pixel>
static bool	accumulate_typed_image(Image<float>& sumimg, double weight,
			Image<Pixel> *srcimg) {
	if (NULL == srcimg) {
		return false;
	}
	int	w = sumimg.size().width();
	int	h = sumimg.size().height();
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			sumimg.writablepixel(x, y)
				= weight * srcimg->pixel(x, y);
		}
	}
	return true;
}

static void	accumulate_image(Image<float>& sumimg, double weight,
			ImagePtr image) {
	if (accumulate_typed_image<unsigned char>(sumimg, weight,
		dynamic_cast<Image<unsigned char>*>(&*image))) return;
	if (accumulate_typed_image<unsigned short>(sumimg, weight,
		dynamic_cast<Image<unsigned short>*>(&*image))) return;
	if (accumulate_typed_image<unsigned int>(sumimg, weight,
		dynamic_cast<Image<unsigned int>*>(&*image))) return;
	if (accumulate_typed_image<unsigned long>(sumimg, weight,
		dynamic_cast<Image<unsigned long>*>(&*image))) return;
	if (accumulate_typed_image<float>(sumimg, weight,
		dynamic_cast<Image<float>*>(&*image))) return;
	if (accumulate_typed_image<double>(sumimg, weight,
		dynamic_cast<Image<double>*>(&*image))) return;
	std::string	msg = stringprintf("cannot accumulate %s image",
				demangle(typeid(*image).name()).c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

ProcessingStep::state	SumStep::do_work() {
	if (!precursorSizesConsistent()) {
		throw std::runtime_error("precursor images are inconsitent");
	}

	// create and initialize an image
	Image<float>	*sumimage = new Image<float>(precursorimage()->size());
	sumimage->fill(0);

	// add all the precursor images 
	steps	ids = precursors();
	std::for_each(ids.begin(), ids.end(),
		[&](int precursorid) {
			ProcessingStepPtr step = byid(precursorid);
			ImageStep *imagestep = dynamic_cast<ImageStep*>(&*step);
			if (imagestep == NULL) {
				return;
			}
			double	weight = imagestep->weight();
			ImagePtr	image = imagestep->image();
			accumulate_image(*sumimage, weight, image);
		}
	);

	return ProcessingStep::failed;
}

std::string	SumStep::what() const {
	return std::string("build the weighted sum of precursors");
}

} // namespace process
} // namespace astro
