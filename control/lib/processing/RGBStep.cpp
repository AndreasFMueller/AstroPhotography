/*
 * RGBStep.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <AstroDebug.h>

namespace astro {
namespace process {

template<typename Pixel, typename SrcPixel>
void	copy_plane(Image<RGB<Pixel> > *result, const Image<SrcPixel> *image,
			int plane, double weight = 1) {
	if ((NULL == result) || (NULL == image)) { return; }
	int	w = result->size().width();
	int	h = result->size().height();
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			SrcPixel	p = image->pixel(x, y);
			switch (plane) {
			case 0: result->writablepixel(x, y).R = p * weight;
				break;
			case 1: result->writablepixel(x, y).G = p * weight;
				break;
			case 2: result->writablepixel(x, y).B = p * weight;
				break;
			default: break;
			}
		}
	}
}

template<typename Pixel>
static ImagePtr	combine(const ImageSequence images, double weights[3]) {
	ImageSize	size = (*images.begin())->size();
	Image<RGB<Pixel> >	*result = new Image<RGB<Pixel> >(size);

	int	plane = 0;
	std::for_each(images.begin(), images.end(),
		[&](ImagePtr image) {
			copy_plane<Pixel, unsigned char>(result,
				dynamic_cast<Image<unsigned char>*>(&*image),
				plane, weights[plane]);
			copy_plane<Pixel, unsigned short>(result,
				dynamic_cast<Image<unsigned short>*>(&*image),
				plane, weights[plane]);
			copy_plane<Pixel, unsigned int>(result,
				dynamic_cast<Image<unsigned int>*>(&*image),
				plane, weights[plane]);
			copy_plane<Pixel, unsigned long>(result,
				dynamic_cast<Image<unsigned long>*>(&*image),
				plane, weights[plane]);
			copy_plane<Pixel, float>(result,
				dynamic_cast<Image<float>*>(&*image),
				plane, weights[plane]);
			copy_plane<Pixel, double>(result,
				dynamic_cast<Image<double>*>(&*image),
				plane, weights[plane]);
			plane++;
		}
	);

	return ImagePtr(result);
}

ProcessingStep::state	RGBStep::do_work() {
	if (3 != precursorCount()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "wrong number of planes: %d",
			precursorCount());
		return ProcessingStep::failed;
	}

	// check consistency of the precursor image sizes
#if 0
	ImageSequence	images = precursorimages();
	ImageSize	size = (*images.begin())->size();
	std::for_each(images.begin(), images.end(),
		[&](ImagePtr image) {
			if (image->size() != size) {
				std::string	msg = stringprintf("image sizes"
					" don't match: %s != %s",
					size.toString().c_str(),
					image->size().toString().c_str());
				debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
				throw std::runtime_error(msg);
			}
		}
	);
#else
	if (!precursorSizesConsistent()) {
		throw std::runtime_error("precursor sizes inconsistent");
	}
#endif

	// get the precursor weights
	double	weights[3];
	steps	precursorsteps = precursors();
	int	i = 0;
	std::for_each(precursorsteps.begin(), precursorsteps.end(),
		[&](int precursorid) {
			if (i < 3) {
				double	w = byid(precursorid)->weight();
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"plane %d (%d) weight %f", i,
					precursorid, w);
				weights[i++] = w;
			}
		}
	);

	// combine the images
	ImageSequence	images = precursorimages();
	if ((_image = combine<float>(images, weights)))
		return ProcessingStep::complete;
#if 0
	if ((_image = combine<double>(images, weights)))
		return ProcessingStep::complete;
#endif

	debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot combine images");

	return ProcessingStep::failed;
}

std::string	RGBStep::what() const {
	return std::string("combine planes into RGB");
}

} // namespace process
} // namespace astro
