/*
 * DeconvolutionStep.cpp -- implementation of the deconvolution step
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <AstroConvolve.h>

namespace astro {
namespace process {

/**
 * \brief Construct a new DeconvolutionStep
 */
DeconvolutionStep::DeconvolutionStep(NodePaths& parent) : ImageStep(parent) {
	_method = std::string("fastvancittert");
}

ProcessingStep::state	DeconvolutionStep::do_fourier(ImagePtr psf, ImagePtr img) {
	FourierDeconvolutionOperator	fdco(psf);
	_image = fdco(img);
	return ProcessingStep::complete;
}

ProcessingStep::state	DeconvolutionStep::do_pseudo(ImagePtr psf, ImagePtr img) {
	PseudoDeconvolutionOperator	pdco(psf);
	pdco.epsilon(epsilon());
	_image = pdco(img);
	return ProcessingStep::complete;
}

ProcessingStep::state	DeconvolutionStep::do_wiener(ImagePtr psf, ImagePtr img) {
	WienerDeconvolutionOperator	wdco(psf);
	wdco.K(K());
	_image = wdco(img);
	return ProcessingStep::complete;
}

ProcessingStep::state	DeconvolutionStep::do_vancittert(ImagePtr psf, ImagePtr img) {
	VanCittertOperator	vc(psf);
	vc.iterations(iterations());
	vc.constrained(true);
	_image = vc(img);
	return ProcessingStep::complete;
}

ProcessingStep::state	DeconvolutionStep::do_fastvancittert(ImagePtr psf, ImagePtr img) {
	FastVanCittertOperator	fvc(psf);
	fvc.iterations(iterations());
	fvc.constrained(true);
	_image = fvc(img);
	return ProcessingStep::complete;
}

ProcessingStep::state	DeconvolutionStep::do_gold(ImagePtr /* psf */, ImagePtr /* img */) {
	debug(LOG_ERR, DEBUG_LOG, 0, "gold deconvolution not defined");
	return ProcessingStep::failed;
}

ProcessingStep::state	DeconvolutionStep::do_work() {
	// build the psf
	ImagePtr	psf;
	if (_psf) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using psf image");
		ImageStep       *j = dynamic_cast<ImageStep*>(&*_psf);
		if (NULL == j) {
			debug(LOG_ERR, DEBUG_LOG, 0, "no psf image found");
			return ProcessingStep::failed;
		}
		psf = j->image();
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "building Gaussian PSF");
		int	s = 5 * stddev();
		if (s < 20) {
			s = 20;
		}
		ImageSize	size(s, s);
		ImagePoint	center = size.center();
		Image<double>	*gauss = new Image<double>(size);
		psf = ImagePtr(gauss);
		double	sum = 0;
		double	n = 2 * stddev() * stddev();
		for (int x = 0; x < s; x++) {
			for (int y = 0; y < s; y++) {
				double	r = hypot(x - center.x(),
						y - center.y());
				double	v = exp(-r * r / n);
				sum += v;
				gauss->pixel(x, y) = v;
			}
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "gaussian values created");
		for (int x = 0; x < s; x++) {
			for (int y = 0; y < s; y++) {
				gauss->pixel(x, y) = gauss->pixel(x, y) / sum;
			}
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "gaussian values normalized");
	}
	if (!psf) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no PSF image ready");
		return ProcessingStep::failed;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "psf %s %s", psf->info().c_str(),
		psf->size().toString().c_str());

	// get the image
	ImagePtr        img = *precursorimages().begin();
	if (!img) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no precursor image");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "precursor image of size %s, %s",
		img->size().toString().c_str(),
		img->info().c_str());

	// perform the deconvolution
	if (method() == std::string("fourier")) {
		return do_fourier(psf, img);
	}
	if (method() == std::string("pseudo")) {
		return do_pseudo(psf, img);
	}
	if (method() == std::string("wiener")) {
		return do_wiener(psf, img);
	}
	if (method() == std::string("vancittert")) {
		return do_vancittert(psf, img);
	}
	if (method() == std::string("fastvancittert")) {
		return do_fastvancittert(psf, img);
	}
	if (method() == std::string("gold")) {
		return do_gold(psf, img);
	}
	return ProcessingStep::failed;
}

std::string	DeconvolutionStep::what() const {
	return std::string("Deconvolution");
}

} // namespace process
} // namespace astro
