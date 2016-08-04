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
#include <AstroFWHM.h>

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
	void	testComponents();
	void	testFWHM();

	CPPUNIT_TEST_SUITE(FWHMTest);
	CPPUNIT_TEST(testComponents);
#if 0
	CPPUNIT_TEST(testFWHM);
#endif
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(FWHMTest);

void	FWHMTest::setUp() {
}

void	FWHMTest::tearDown() {
}

void	FWHMTest::testComponents() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testComponents() begin");
	int	w = 700;
	int	h = 500;
	Image<unsigned short>	*image = new Image<unsigned short>(w, h);
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			double	u = (x + 100) * y;
			double	v = (y + 1.) / (x + 101.);
			double	f = trunc(165 * (0.5 + sin(u / 4000.) * sin(v * 5.)));
			if (f > 240) {
				image->pixel(x, y) = 240;
			} else if (f < 0) {
				image->pixel(x, y) = 0;
			} else {
				image->pixel(x, y) = f;
			}
		}
	}
	ImagePtr	imageptr(image);
	FITSout	out("tmp/fwhm.fits");
	out.setPrecious(false);
	out.write(imageptr);

	// analyze components
	fwhm::ComponentDecomposer	decomposer(imageptr, true);

	// info abou components
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d components found",
		decomposer.numberOfComponents());

	std::list<image::fwhm::ComponentInfo>::const_iterator	ci;
	for (ci = decomposer.components().begin();
		ci != decomposer.components().end(); ci++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", ci->toString().c_str());
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testComponents() end");
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
