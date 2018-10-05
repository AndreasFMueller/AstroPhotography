/*
 * BlurrTest.cpp -- template for tests
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroFilterfunc.h>
#include <AstroAdapter.h>
#include <AstroIO.h>
#include <Blurr.h>

using namespace astro::image;
using namespace astro::adapter;

namespace astro {
namespace test {

class BlurrTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testBlurr();
	void	testBlurr2();
	void	testBlurr3();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(BlurrTest);
	CPPUNIT_TEST(testBlurr);
	CPPUNIT_TEST(testBlurr2);
	CPPUNIT_TEST(testBlurr3);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BlurrTest);

void	BlurrTest::setUp() {
}

void	BlurrTest::tearDown() {
}

static void	performTest(const ImageSize& size, Blurr& blurr) {
	// initialize a black image
	Image<double>	image(size);
	for (int x = 0; x < size.width(); x++) {
		for (int y = 0; y < size.height(); y++) {
			image.pixel(x,y) = 0;
		}
	}

	// add some image information
	unsigned int	n = size.getPixels() / 10000;
	for (unsigned int i = 0; i < n; i++) {
		int	xs = random() % size.width();
		int	ys = random() % size.height();
		int	r = random() % 6;
		int	xmin = xs - r; if (xs < 0) { xs = 0; }
		int	xmax = xs + r; if (xs >= size.width()) {
						xs = size.width();
					}
		int	ymin = ys - r; if (ys < 0) { ys = 0; }
		int	ymax = ys + r; if (ys >= size.height()) {
						ys = size.height();
					}
		for (int x = xmin; x < xmax; x++) {
			for (int y = ymin; y < ymax; y++) {
				if (hypot(x - xs, y - ys) < r) {
					image.pixel(x, y) = 1;
				}
			}
		}
	}

	// blurr the image
	Image<double>	blurredimage = blurr(image);


	// build the name
	std::string	filename = stringprintf("blurr-%s-%.0f-%.0f.fits",
				size.toString().c_str(),
				blurr.radius(), blurr.innerradius());

	// write the result
	io::FITSoutfile<double>	out(filename);
	out.setPrecious(false);
	out.write(blurredimage);
}

void	BlurrTest::testBlurr() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBlurr() begin");
	ImageSize	size(1024, 1024);
	Blurr	blurr(30, 12);
	performTest(size, blurr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBlurr() end");
}

void	BlurrTest::testBlurr2() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBlurr2() begin");
	ImageSize	size(640, 480);
	Blurr	blurr(30, 12);
	performTest(size, blurr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBlurr2() end");
}

void	BlurrTest::testBlurr3() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBlurr3() begin");
	ImageSize	size(1920, 1080);
	Blurr	blurr(30, 12);
	performTest(size, blurr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBlurr3() end");
}

#if 0
void	BlurrTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
