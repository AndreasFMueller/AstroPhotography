/*
 * FWHMTest.cpp -- test pixel conversions
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroImage.h>
#include <AstroIO.h>
#include <AstroFilter.h>
#include <AstroFilterfunc.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <AstroDebug.h>

using namespace astro::image;
using namespace astro::image::filter;
using namespace astro::io;

namespace astro {
namespace test {

class FWHMTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testFWHM();

	CPPUNIT_TEST_SUITE(FWHMTest);
	CPPUNIT_TEST(testFWHM);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(FWHMTest);

void	FWHMTest::setUp() {
}

void	FWHMTest::tearDown() {
}

void	FWHMTest::testFWHM() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testFWHM() begin");
	// read the test image
	FITSin	in("testimages/g014.fits");
	ImagePtr	image = in.read();

	ImagePoint	center(458, 486 - 165);
	double	fwhm = focusFWHM(image, center, 20);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "FWHM = %f", fwhm);
	CPPUNIT_ASSERT(fwhm == 5);

	center = ImagePoint(352, 486 - 216);
	fwhm = focusFWHM(image, center, 20);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "FWHM = %f", fwhm);
	CPPUNIT_ASSERT(fwhm == 24.5);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testFWHM() end");
}


} // namespace test
} // namespace astro
