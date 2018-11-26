/*
 * LayerImageStep.cpp -- implementation of the LayerImageStep
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <sstream>

namespace astro {
namespace process {

LayerImageStep::LayerImageStep(NodePaths& parent) : ImageStep(parent) {
}

/**
 * \brief Copy a single image into a plane
 */
template<typename Pixel, int n, typename SrcPixel>
bool	copy_plane_src(Image<Multiplane<Pixel, n> >*resultimg, int i,
			Image<SrcPixel> *srcimg) {
	if (NULL == resultimg) {
		return false;
	}
	if (NULL == srcimg) {
		return false;
	}
	int	w = srcimg->size().width();
	int	h = srcimg->size().height();
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			resultimg->writablepixel(x, y).p[i]
				= srcimg->pixel(x, y);
		}
	}
	return true;
}

/**
 * \brief copy all planes to a multiplane image
 */
template<typename Pixel, int n>
ImagePtr	copy_planes(const ImageSequence& images) {
	// make sure the number of planes is right
	if (n != images.size()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "wrong nmbr of images %d != %d",
			n, images.size());
		return ImagePtr();
	}

	// get the size from the first image in the sequence
	ImageSize	size = (*images.begin())->size();

	// create the result image
	Image<Multiplane<Pixel, n> >	*resultimg
		= new Image<Multiplane<Pixel, n> >(size);
	ImagePtr	result(resultimg);

	// now copy all the planes
	int	i = 0;
	std::for_each(images.begin(), images.end(),
		[&](ImagePtr input) {
			copy_plane_src<Pixel, n, unsigned char>(resultimg, i,
				dynamic_cast<Image<unsigned char>*>(&*input));
			copy_plane_src<Pixel, n, unsigned short>(resultimg, i,
				dynamic_cast<Image<unsigned short>*>(&*input));
			copy_plane_src<Pixel, n, unsigned int>(resultimg, i,
				dynamic_cast<Image<unsigned int>*>(&*input));
			copy_plane_src<Pixel, n, unsigned long>(resultimg, i,
				dynamic_cast<Image<unsigned long>*>(&*input));
			copy_plane_src<Pixel, n, float>(resultimg, i,
				dynamic_cast<Image<float>*>(&*input));
			copy_plane_src<Pixel, n, double>(resultimg, i,
				dynamic_cast<Image<double>*>(&*input));
			i++;
		}
	);

	return result;
}

/**
 * \brief Do the processing work
 */
ProcessingStep::state	LayerImageStep::do_work() {
	ImageSize	size = precursorimage()->size();

	// make sure all precursors have the same size
	ImageSequence	images = precursorimages();
	std::for_each(images.begin(), images.end(),
		[&](ImagePtr& img) {
			if (img->size() != size) {
				std::string	msg = stringprintf("image sizes"
					" differe %s != %s",
					img->size().toString().c_str(),
					size.toString().c_str());
				debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
				throw std::runtime_error(msg);
			}
		}
	);

	// copy the layers
	switch (images.size()) {
	case 1: _image = copy_planes<float, 1>(images); break;
	case 2: _image = copy_planes<float, 2>(images); break;
	case 3: _image = copy_planes<float, 3>(images); break;
	case 4: _image = copy_planes<float, 4>(images); break;
	case 5: _image = copy_planes<float, 5>(images); break;
	case 6: _image = copy_planes<float, 6>(images); break;
	case 7: _image = copy_planes<float, 7>(images); break;
	case 8: _image = copy_planes<float, 8>(images); break;
	case 9: _image = copy_planes<float, 9>(images); break;
	default:
		debug(LOG_ERR, DEBUG_LOG, 0, "wrong number of planes: %d",
			images.size());
		return ProcessingStep::failed;
	}

	// copying layers complete
	return ProcessingStep::complete;
}

/**
 * \brief Info about the step
 */
std::string	LayerImageStep::what() const {
	std::ostringstream	out;
	out << "combine precursor images into layers";
	return out.str();
}

} // namespace process
} // namespace astro
