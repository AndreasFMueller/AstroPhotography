/*
 * StereographicStereographicChartTest.cpp
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

class StereographicChartTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testImage();

	CPPUNIT_TEST_SUITE(StereographicChartTest);
	CPPUNIT_TEST(testImage);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(StereographicChartTest);

void	StereographicChartTest::setUp() {
}

void	StereographicChartTest::tearDown() {
}

void	StereographicChartTest::testImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testImage() begin");

	// get the center, in the constellation andromeda
	RaDec	center;

	// center
	center.ra().hours(0);
	center.dec().degrees(44.);

	// star catalog
	CatalogPtr	catalog = CatalogFactory::get(CatalogFactory::Combined,
					"/usr/local/starcatalogs");

	// point spread function
	TurbulencePointSpreadFunction	psf(1.5);

	// build the factory
	//double	limit_mag = 20;
	double	limit_mag = 6; // for M31
	StereographicChartFactory	factory(catalog, psf, limit_mag,
						4, 7);

	// create the image
	unsigned int	number_of_images = 1;
	for (unsigned int h = 0; h < number_of_images; h++) {
		StereographicChart	chart = factory.chart(center, 1024);
		center.ra().degrees(h);
		astro::image::ImagePtr	image = chart.image();
		std::string	filename
			= stringprintf("tmp/stereochart-%03u.fits", h);
		astro::io::FITSout	out(filename);
		out.setPrecious(false);
		out.write(image);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testImage() end");
}

} // namespace test
} // namespace astro
