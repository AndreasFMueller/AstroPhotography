/*
 * ImageNormalizer.cpp -- normalize onto star chart at the center of the image
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroChart.h>
#include <AstroIO.h>
#include <AstroAdapter.h>

using namespace astro::image;
using namespace astro::image::transform;
using namespace astro::io;
using namespace astro::adapter;

namespace astro {
namespace catalog {

ImageNormalizer::ImageNormalizer(ChartFactory& factory) : _factory(factory) { }

/**
 * \brief Auxiliary function to extract current center coordinates from image
 */
static RaDec	get_center(ImagePtr image) {
	RaDec	center;
	std::string	v = image->getMetadata("RACENTR").getValue();
	center.ra().hours(std::stod(v));
	v = image->getMetadata("DECCENTR").getValue();
	center.dec().degrees(std::stod(v));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "current image center: %s",
		center.toString().c_str());
	return center;
}

static void	writefile(const std::string& filename,
			const ConstImageAdapter<double>& image) {
	FITSoutfile<double>	out(filename);
	out.setPrecious(false);
	out.write(Image<double>(image));
}

static void	writefile(const std::string& filename, ImagePtr image) {
	// build an image 
	Image<double>	*imagep = dynamic_cast<Image<double> *>(&*image);
	if (NULL == imagep) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s not a double image",
			filename.c_str());
		return;
	}

	// write the projected image
	FITSoutfile<double>	out(filename);
	out.setPrecious(false);
	out.write(*imagep);
}

/**
 * \brief Compute the true 
 *
 * \param image			
 * \param initialprojection	initial value for the projection
 */
RaDec	ImageNormalizer::operator()(ImagePtr image, Projection& projection) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "normalizing %s image",
		image->size().toString().c_str());
	DoubleAdapter	doubleimage(image);

	// get the geometry of the image
	ImageGeometry	geometry(*image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image geometry: %s",
		geometry.toString().c_str());

	// we will try to figure out a chart, and we start with a chart
	// exactly the same size as the image
	ImageSize	chartsize = image->size();

	// find the current central coordinates of the image
	RaDec	center = get_center(image);

	int	iterations = 0;
	do {
		// we need point on the cart that is mapped to 0.
		Transform	inverse = projection.inverse();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "inverse transform: %s",
			inverse.toString().c_str());
		Point	offset = inverse(Point(0, 0));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "offset = %s",
			offset.toString().c_str());
		Point	roffset(2 * offset.x() / chartsize.width(),
				2 * offset.y() / chartsize.height());
		SkyRectangle	chartrectangle(center, geometry);
		RaDec	newcenter = chartrectangle.inverse(roffset);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "new image center: %s",
			newcenter.toString().c_str());

		// the new center also gives us a new chart rectangle:
		chartrectangle = SkyRectangle(newcenter, geometry);

		// the projection now no longer needs the translation component
		// so we remove that
		projection[2] = 0;
		projection[5] = 0;
		inverse = projection.inverse();

		// we need a projection centered at the centers of image and
		// chart,
		CenteredProjection	centeredprojection(chartsize.center(),
					image->size().center(), projection);

		// Now change the chart rectangle to the new center,
		// but still using the same geometry.
		debug(LOG_DEBUG, DEBUG_LOG, 0, "centered projection: %s",
			centeredprojection.toString().c_str());

		// Compute a rectangle that is large enough so that the
		// transformed image finds place in it. To get such a
		// rectangle, we compute the the relative coordinates of
		// the transformed corners. If we already had the right
		// dimensions for the chart rectangle, we would have
		// one of the relative coordinates always +-1. So the
		// relative coordinates tell us how to resize the chart
		// image size
		std::set<Point>	corners;
		corners.insert(inverse(image->size().upperright()));
		corners.insert(inverse(image->size().lowerright()));
		corners.insert(inverse(image->size().upperleft()));
		corners.insert(inverse(image->size().lowerleft()));
		Size	size(corners);
		chartsize = ImageSize(2 * (int)(size.width() / 2),
				2 * (int)(size.height() / 2));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "new size: %s, chartsize: %s",
			size.toString().c_str(), chartsize.toString().c_str());

		// now recompute chart geometry
		ImageGeometry	chartgeometry(chartsize, geometry.focallength(),
					geometry.pixelsize());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "chartgeometry: %s",
			chartgeometry.toString().c_str());

		// compute a chart for that larger rectangle
		Chart	chart = _factory.chart(newcenter, chartgeometry);
		if (debuglevel >= LOG_DEBUG) {
			writefile(stringprintf("foo-%d-chart.fits",
				iterations), chart.image());
		}

		// we use the chart as the base
		DoubleAdapter	doublechart(chart.image());
		Analyzer	analyzer(doublechart, 512, 512);

		ProjectionAdapter<double>	projected(
			doublechart.getSize(), doubleimage, projection);
		// also write the transformed image 
		if (debuglevel >= LOG_DEBUG) {
			writefile(stringprintf("foo-%d-projected.fits",
				iterations), projected);
		}

		// compute the residuals
		std::vector<Residual>	residuals = analyzer(projected);

		// convert the residuals to the right coordinate system ???
#if 1
		std::vector<Residual>::iterator	r;
		for (r = residuals.begin(); r != residuals.end(); r++) {
			r->second = -inverse(r->second);
		}
#endif

		// try to match the larger rectangle inside the chart
		ProjectionCorrector	corrector(doublechart.getSize(),
					doubleimage.getSize(), projection);
		projection = corrector.corrected(residuals);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "corrected projection: %s",
			projection.toString().c_str());

		// write the projected image
		if (debuglevel >= LOG_DEBUG) {
			writefile(stringprintf("foo-%d-corrected.fits",
				iterations),
				ProjectionAdapter<double>(doublechart.getSize(),
					doubleimage, projection));
		}

		// use the new center
		center = newcenter;

		// count the iterations
		iterations++;
	} while (iterations < 10);

	// give back the new center
	return center;
}

} // namespace catalog
} // namespace astro
