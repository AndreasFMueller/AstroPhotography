/*
 * ImageNormalizerTest.cpp -- test the image normalizer
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroChart.h>
#include <AstroIO.h>
#include <AstroDebug.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

using namespace astro::image::transform;
using namespace astro::io;

namespace astro {
namespace catalog {

class ImageNormalizerTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testImageNormalizer();

	CPPUNIT_TEST_SUITE(ImageNormalizerTest);
	CPPUNIT_TEST(testImageNormalizer);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ImageNormalizerTest);

void	ImageNormalizerTest::setUp() {
}

void	ImageNormalizerTest::tearDown() {
}

void	ImageNormalizerTest::testImageNormalizer() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testImageNormalizer() begin");

	// create a Chart factory
	Catalog	catalog("/usr/local/starcatalogs");
	TurbulencePointSpreadFunction	psf(2);
	ChartFactory	factory(catalog, psf, 14, 100);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "chart factory created");

	// create an Image Normalizer
	ImageNormalizer	normalizer(factory);
	
	// prepare the initial transformation
	Projection	projection(M_PI * 162 / 180, Point(838, 182), 0.98);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "projection: %s",
		projection.toString().c_str());

	// get the image from the input file
	FITSin  in("testimages/andromeda-base.fits");
	ImagePtr	imageptr = in.read();
	Image<unsigned char>	*image
		= dynamic_cast<Image<unsigned char> *>(&*imageptr);
	CPPUNIT_ASSERT(NULL != image);

	// apply the normalizer to the 
	debug(LOG_DEBUG, DEBUG_LOG, 0, "apply normalizer");
	RaDec	center = normalizer(imageptr, projection);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "true center: %s",
		center.toString().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "transformation: %s",
		projection.toString().c_str());

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testImageNormalizer() end");
}


} // namespace catalog
} // namespace astro
