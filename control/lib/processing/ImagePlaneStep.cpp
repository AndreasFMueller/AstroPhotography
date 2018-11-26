/*
 * ImagePlaneStep.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil 
 */
#include <AstroProcess.h>

namespace astro {
namespace process {

template<typename Pixel, int n>
ImagePtr	extract_multiplane(Image<Multiplane<Pixel, n> >*image, int i) {
	if ((i >= n) || (NULL != image)) {
		return ImagePtr();
	}

	// create the target image
	int	w = image->size().width();
	int	h = image->size().height();
	Image<Pixel>	*outimg = new Image<Pixel>(image->size());

	// copy the data
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			outimg->writablepixel(x, y) = image->pixel(x, y).p[i];
		}
	}

	// return the image
	return ImagePtr(outimg);
}

template<typename Pixel>
ImagePtr	extract_rgb(Image<RGB<Pixel> >*image, int i) {
	if ((i >= 3) || (NULL != image)) {
		return ImagePtr();
	}
	// create the target image
	int	w = image->size().width();
	int	h = image->size().height();
	Image<Pixel>	*outimg = new Image<Pixel>(image->size());

	// copy the data
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			RGB<Pixel>	p = image->pixel(x, y);
			switch (i) {
			case 0:	outimg->writablepixel(x, y) = p.R; break;
			case 1:	outimg->writablepixel(x, y) = p.G; break;
			case 2:	outimg->writablepixel(x, y) = p.B; break;
			default:
				break;
			}
		}
	}

	// return the image
	return ImagePtr(outimg);
}

template<typename Pixel>
ImagePtr	extract(ImagePtr image, int i) {
	ImagePtr	result;
	if ((result = extract_rgb(dynamic_cast<Image<RGB<Pixel> >*>(&*image),
		i))) return result;
	if ((result = extract_multiplane(dynamic_cast<Image<Multiplane<Pixel,
		1> >*>(&*image), i))) return result;
	if ((result = extract_multiplane(dynamic_cast<Image<Multiplane<Pixel,
		2> >*>(&*image), i))) return result;
	if ((result = extract_multiplane(dynamic_cast<Image<Multiplane<Pixel,
		3> >*>(&*image), i))) return result;
	if ((result = extract_multiplane(dynamic_cast<Image<Multiplane<Pixel,
		4> >*>(&*image), i))) return result;
	if ((result = extract_multiplane(dynamic_cast<Image<Multiplane<Pixel,
		5> >*>(&*image), i))) return result;
	if ((result = extract_multiplane(dynamic_cast<Image<Multiplane<Pixel,
		6> >*>(&*image), i))) return result;
	if ((result = extract_multiplane(dynamic_cast<Image<Multiplane<Pixel,
		7> >*>(&*image), i))) return result;
	if ((result = extract_multiplane(dynamic_cast<Image<Multiplane<Pixel,
		8> >*>(&*image), i))) return result;
	if ((result = extract_multiplane(dynamic_cast<Image<Multiplane<Pixel,
		9> >*>(&*image), i))) return result;
	return ImagePtr();
}

ProcessingStep::state	ImagePlaneStep::do_work() {
	if ((_image = extract<unsigned char>(precursorimage(), _n)))
		return ProcessingStep::complete;
	if ((_image = extract<unsigned short>(precursorimage(), _n)))
		return ProcessingStep::complete;
	if ((_image = extract<unsigned int>(precursorimage(), _n)))
		return ProcessingStep::complete;
	if ((_image = extract<unsigned long>(precursorimage(), _n)))
		return ProcessingStep::complete;
	if ((_image = extract<float>(precursorimage(), _n)))
		return ProcessingStep::complete;
	if ((_image = extract<double>(precursorimage(), _n)))
		return ProcessingStep::complete;

	return ProcessingStep::failed;
}

std::string	ImagePlaneStep::what() const {
	return stringprintf("extract plane %d", _n);
}

} // namespace process
} // namespace astro
