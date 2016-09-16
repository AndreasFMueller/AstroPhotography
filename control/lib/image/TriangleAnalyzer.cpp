/*
 * TriangleAnalyzer.cpp -- Transform analyzer that uses the triangle
 *                         matching method
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTransform.h>

namespace astro {
namespace image {
namespace transform {

TriangleAnalyzer::TriangleAnalyzer(const ConstImageAdapter<double>& image,
	int numberofstars, int searchradius) {
	factory.numberofstars(numberofstars);
	factory.radius(searchradius);
	fromtriangles = factory.get(image);
}

TriangleAnalyzer::TriangleAnalyzer(ImagePtr image,
	int numberofstars, int searchradius) {
	factory.numberofstars(numberofstars);
	factory.radius(searchradius);
	fromtriangles = factory.get(image);
}

Transform	TriangleAnalyzer::transform(const ConstImageAdapter<double>& image) const {
	TriangleSet	totriangles = factory.get(image);
	return fromtriangles.closest(totriangles);
}

Transform	TriangleAnalyzer::transform(ImagePtr image) const {
	TriangleSet	totriangles = factory.get(image);
	return fromtriangles.closest(totriangles);
}

} // namespace transform
} // namespace image
} // namespace astro
