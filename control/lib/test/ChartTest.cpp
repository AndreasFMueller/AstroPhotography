/*
 * ChartTest.cpp -- tests for the Chart class
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroChart.h>
#include <AstroDebug.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <AstroIO.h>

using namespace astro::catalog;
using namespace astro::image;

namespace astro {
namespace test {

class ChartTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testImage();

	CPPUNIT_TEST_SUITE(ChartTest);
	CPPUNIT_TEST(testImage);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ChartTest);

void	ChartTest::setUp() {
}

void	ChartTest::tearDown() {
}

void	ChartTest::testImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testImage() begin");

	// get the center, in the constellation andromeda
	RaDec	center;
	// get the center, in the constellation andromeda
	//center.ra().hours(42./60 + 44./3600);
	//center.dec().degrees(41 + 16./60 + 10./3600);
	// Orion
	//center.ra().hours(4 + 43/60. + 25/3600.);
	//center.dec().degrees(-10 - 58./60 -43./36000);
	// M13
	//center.ra().hours(16 + 41./60 + 41.44/3600);
	//center.dec().degrees(36 + 27./60 + 36.9/3600);
	// small magellanic cloud
	//center.ra().hours(0 + 51./60);
	//center.dec().degrees(-73 - 6./60);

	// Deneb
	center.ra().hours(20. + 41./60 + 25.9/3600);
	center.dec().degrees(45 + 16./60 + 49./3600);

	//  36UMa
	//center.ra().hours(10. + 30./60 + 37.6/3600);
	//center.dec().degrees(55 + 58./60 + 50.0/3600);

	// M31
	center.ra().hours(0. + 42./60 + 44.3/3600);
	center.dec().degrees(41 + 16./60 + 9./3600);

	// create chart object
	// SX MC26C 50mm 
	//DiffractionChartFactory	chart(ImageSize(3900, 2616), center, 0.050, 0.00000605);
	// SX MC26C 135mm 
	TurbulenceChartFactory	chartfactory(ImageSize(3900, 2616), center, 0.135, 0.00000605);
	// SX MC26, primary focus
	//TurbulenceChartFactory	chart(ImageSize(3900, 2616), center, 0.560, 0.00000605);
	// SBIG 16803, Cassegrain focus
	//TurbulenceChartFactory	chart(ImageSize(4096, 4096), center, 2.800, 0.000015);
	chartfactory.maxradius(7);
	//chart.aperture(0.280);
	chartfactory.turbulence(2);
	chartfactory.scale(500);
	//chart.logarithmic(true);

	// extract the window 
	SkyWindow	window = chartfactory.getWindow();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get stars from window %s",
		window.toString().c_str());

	//double	limit_mag = 20;
	double	limit_mag = 14; // for M31
	chartfactory.scale(100); // M31

	// retrieve Hipparcos stars
	Catalog	catalog("/usr/local/starcatalogs");
	Catalog::starsetptr	stars = catalog.find(window,
					MagnitudeRange(-30, limit_mag));

	// report number of stars found
	debug(LOG_DEBUG, DEBUG_LOG, 0, "chart contains %u stars",
		stars->size());

	// create the image
	chartfactory.draw(stars);
	Chart	chart = chartfactory.chart();
	astro::image::ImagePtr	image = chart.image();
	astro::io::FITSout	out("chart.fits");
	out.setPrecious(false);
	out.write(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testImage() end");
}

} // namespace test
} // namespace astro
