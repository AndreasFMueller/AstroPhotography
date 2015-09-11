/*
 * ChartFactoryBase.cpp -- base class for chart factories
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroChart.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroIO.h>
#include <typeinfo>

using namespace astro::image;

namespace astro {
namespace catalog {

void	ChartFactoryBase::draw(Image<double>& image, const Point& p,
		const Star& star) const {
	// compute the radius of the star
	double	I;
	if (_logarithmic) {
		I = 1 - star.mag() / 20;
	} else {
		I = pow(10., -star.mag() / 5);
	}
	I *= _scale;
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "mag = %f, I = %f", star.mag(), I);

	// get the coordinates of the point
	int	x = floor(p.x());
	int	y = floor(p.y());
	double	wx = p.x() - x;
	double	wy = p.y() - y;

	bool	havedrawnsomething = false;
	if (image.getFrame().contains(ImagePoint(x, y))) {
		image.pixel(x    , y    ) += I * (1 - wx) * (1 - wy);
		havedrawnsomething = true;
	}
	if (image.getFrame().contains(ImagePoint(x + 1, y))) {
		image.pixel(x + 1, y    ) += I *      wx  * (1 - wy);
		havedrawnsomething = true;
	}
	if (image.getFrame().contains(ImagePoint(x, y + 1))) {
		image.pixel(x    , y + 1) += I * (1 - wx) *      wy ;
		havedrawnsomething = true;
	}
	if (image.getFrame().contains(ImagePoint(x + 1, y + 1))) {
		image.pixel(x + 1, y + 1) += I *      wx  *      wy ;
		havedrawnsomething = true;
	}
	if (star.mag() > 6) {
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "star %s at %s %s, value = %f",
		star.toString().c_str(), p.toString().c_str(),
		(havedrawnsomething) ? "drawn" : "skipped", I);
}

void	ChartFactoryBase::limit(Image<double>& image, double limit) const {
	int	counter = 0;
	for (int x = 0; x < image.size().width(); x++) {
		for (int y = 0; y < image.size().height(); y++) {
			double	value = image.pixel(x, y);
			if (value > limit) {
				image.pixel(x, y) = limit;
				counter++;
			} else {
				image.pixel(x, y) = value;
			}
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d pixels limited to %f (scale = %f)",
		counter, limit, _scale);
}

void	ChartFactoryBase::spread(Image<double>& image, int morepixels,
		const ImageGeometry& geometry) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "apply point spread function %s",
		typeid(pointspreadfunction).name());
	// we don't need to to anything if the point spread function is
	// a DiracPointSpreadFunction
	if (0 == strcmp(typeid(pointspreadfunction).name(),
		typeid(astro::catalog::DiracPointSpreadFunction).name())) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "skip Dirac PSF");
		return;
	}

	// create an image with more pixels around the border
	ImageSize	imgsize(image.size().width() + 2 * morepixels,
				image.size().height() + 2 * morepixels);
	ImagePoint	imgoffset(morepixels, morepixels);
	adapter::BorderAdapter<double>	imgborder(imgsize, imgoffset, image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "border adapter for image has size %s",
		imgborder.getSize().toString().c_str());

	// for debugging, write the PSF
	io::FITSout	outimage("tmp/image.fits");
	outimage.setPrecious(false);
	ImagePtr	imgimage(new Image<double>(imgborder));
	outimage.write(imgimage);

	// create an image for the point spread function
	ImageSize	psfsize(2 * morepixels, 2 * morepixels);
	ImagePoint	psfoffset(morepixels, morepixels);
	PointSpreadFunctionAdapter	psfadapter(psfsize, psfoffset,
			geometry.angularpixelsize(), pointspreadfunction);

	// embedd the point spread function in a larger border adapter
	adapter::BorderAdapter<double>	psfborder(imgsize, ImagePoint(0, 0),
						psfadapter);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "border adapter for PSF has size %s",
		psfborder.getSize().toString().c_str());

	// for debugging, write the PSF
	io::FITSout	outpsf("tmp/psf.fits");
	outpsf.setPrecious(false);
	ImagePtr	psfimage(new Image<double>(psfborder));
	outpsf.write(psfimage);

	// now perform the convolution
	ConvolutionResult	i(imgborder, ImagePoint(0, 0));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image transformed");
	ConvolutionResult	p(psfborder, ImagePoint(0, 0) /* psfoffset */);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "psf transformed");
	ConvolutionResultPtr	c = i * p;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "convolution complete");
	ImagePtr        imageptr = c->image();
	Image<double>	*img = dynamic_cast<Image<double> *>(&*imageptr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image transformed back");

	io::FITSout	out2("tmp/blubb2.fits");
	out2.setPrecious(false);
	out2.write(imageptr);

	// extract the right part
	ImageRectangle	rectangle(ImagePoint(2 * morepixels, 2 * morepixels),
				image.getSize());
	adapter::WindowAdapter<double>	result(*img, rectangle);

	io::FITSout	out3("tmp/blubb3.fits");
	out3.setPrecious(false);
	ImagePtr	image3ptr(new Image<double>(result));
	out3.write(image3ptr);

	// copy the pixels from the convolution to the image
	copy(image, result);
}

} // namespace catalog
} // namespace astro
