/*
 * AdapterTest.cpp -- test adapters to conversions
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroAdapter.h>
#include <AstroConvolve.h>
#include <AstroTransform.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroIO.h>
#include <AstroDebug.h>
#include <iostream>

using namespace astro::image;
using namespace astro::adapter;

namespace astro {
namespace test {

class AdapterTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();

	void	testIdentity();
	void	testTiling();
	void	testFundamental();
	void	testShift();
	void	testRoll();
	void	testRollAdapter();
	void	testAmplifier();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(AdapterTest);
	CPPUNIT_TEST(testIdentity);
	CPPUNIT_TEST(testTiling);
	CPPUNIT_TEST(testFundamental);
	CPPUNIT_TEST(testShift);
	CPPUNIT_TEST(testRoll);
	CPPUNIT_TEST(testRollAdapter);
	CPPUNIT_TEST(testAmplifier);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(AdapterTest);

void	AdapterTest::setUp() {
}

void	AdapterTest::tearDown() {
}

void	AdapterTest::testIdentity() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testIdentity() begin");
	Image<unsigned char>	image(47, 53);
	for (int x = 0; x < image.getSize().width(); x++) {
		for (int y = 0; y < image.getSize().height(); y++) {
			unsigned char	v = (x + y) % 256;
			image.pixel(x, y) = v;
		}
	}
	IdentityAdapter<unsigned char>	identity(image);
	for (int x = 0; x < image.getSize().width(); x++) {
		for (int y = 0; y < image.getSize().height(); y++) {
			unsigned char	v = (x + y) % 256;
			CPPUNIT_ASSERT(identity.pixel(x, y) == v);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testIdentity() end");
}

void	AdapterTest::testFundamental() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testFundamental() begin");
	Image<unsigned char>	image(47, 53);
	for (int x = 0; x < image.getSize().width(); x++) {
		for (int y = 0; y < image.getSize().height(); y++) {
			unsigned char	v = (x + y) % 256;
			image.pixel(x, y) = v;
		}
	}
	FundamentalAdapter<unsigned char>	fundamental(image);
	for (int x = 0; x < 2 * image.getSize().width(); x++) {
		for (int y = 0; y < 2 * image.getSize().height(); y++) {
			unsigned char	v = (x + y) % 256;
			//debug(LOG_DEBUG, DEBUG_LOG, 0, "%d,%d %d %d", x, y,
			//	(int)v, fundamental.pixel(x, y));
			if ((x >= image.getSize().width())
				|| (y >= image.getSize().height())) {
				CPPUNIT_ASSERT(fundamental.pixel(x, y) == 0);
			} else {
				CPPUNIT_ASSERT(fundamental.pixel(x, y) == v);
			}
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testFundamental() end");
}

void	AdapterTest::testTiling() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testTiling() begin");
	Image<unsigned char>	image(47, 53);
	for (int x = 0; x < image.getSize().width(); x++) {
		for (int y = 0; y < image.getSize().height(); y++) {
			unsigned char	v = (x + y) % 256;
			image.pixel(x, y) = v;
		}
	}
	TilingAdapter<unsigned char>	tiling(image);
	for (int x = 0; x < 2 * image.getSize().width(); x++) {
		for (int y = 0; y < 2 * image.getSize().height(); y++) {
			unsigned char	v = ((x % 47) + (y % 53)) % 256;
			//debug(LOG_DEBUG, DEBUG_LOG, 0, "%d,%d %d %d", x, y,
			//	(int)v, tiling.pixel(x, y));
			CPPUNIT_ASSERT(tiling.pixel(x, y) == v);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testTiling() end");
}

void	AdapterTest::testShift() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testShift() begin");
	Image<unsigned char>	image(47, 53);
	for (int x = 0; x < image.getSize().width(); x++) {
		for (int y = 0; y < image.getSize().height(); y++) {
			unsigned char	v = (x + y) % 256;
			image.pixel(x, y) = v;
		}
	}
	ImagePoint	offset(3, 4);
	ShiftAdapter<unsigned char>	shift(image, offset);
	for (int x = 0; x < image.getSize().width(); x++) {
		for (int y = 0; y < image.getSize().height(); y++) {
			int	x0 = x + 3;
			int	y0 = y + 4;
			unsigned char	v = 0;
			if ((0 <= x0) && (x0 < 47) && (0 <= y0) && (y0 < 53)) {
				v = (x0 + y0) % 256;
			}
			//debug(LOG_DEBUG, DEBUG_LOG, 0, "%d,%d %d,%d %d %d",
			//	x, y, x0, y0, (int)v, shift.pixel(x, y));
			CPPUNIT_ASSERT(shift.pixel(x, y) == v);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testShift() end");
}

void	AdapterTest::testRoll() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRoll() begin");
	Image<unsigned char>	image(47, 53);
	for (int x = 0; x < image.getSize().width(); x++) {
		for (int y = 0; y < image.getSize().height(); y++) {
			unsigned char	v = (x + y) % 256;
			image.pixel(x, y) = v;
		}
	}
	ImagePoint	offset(3, 4);
	RollAdapter<unsigned char>	roll(image, offset);
	for (int x = 0; x < image.getSize().width(); x++) {
		for (int y = 0; y < image.getSize().height(); y++) {
			int	x0 = x + 3;
			int	y0 = y + 4;
			unsigned char	v = ((x0 % 47) + (y0 % 53)) % 256;
			CPPUNIT_ASSERT(roll.pixel(x, y) == v);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRoll() end");
}

void	AdapterTest::testRollAdapter() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRollAdapter() begin");
	Image<unsigned char>	*image = new Image<unsigned char>(47, 53);
	ImagePtr	imageptr(image);
	for (int x = 0; x < 47; x++) {
		for (int y = 0; y < 53; y++) {
			unsigned char	v = (x + y) % 256;
			image->pixel(x, y) = v;
		}
	}
	DoubleAdapter	doubleimage(imageptr);
	transform::RollAdapter<double>	roll(doubleimage, Point(0.3, 1.6));
	double	w00 = 0.3 * 0.6;
	double	w10 = 0.7 * 0.6;
	double	w01 = 0.3 * 0.4;
	double	w11 = 0.7 * 0.4;
	TilingAdapter<unsigned char>	tiling(*image);
	for (int x = 0; x < 47; x++) {
		for (int y = 0; y < 53; y++) {
			double	v =	w00 * tiling.pixel(x, y + 1) +
					w10 * tiling.pixel(x + 1, y + 1) +
					w01 * tiling.pixel(x, y + 2) +
					w11 * tiling.pixel(x + 1, y + 2);
			CPPUNIT_ASSERT(fabs(roll.pixel(x, y) - v) < 0.01);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRollAdapter() end");
}

void	AdapterTest::testAmplifier() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAmplifier() begin");
	ImageSize	size(640, 480);
	ImagePoint	center(-20, 400);
	AmplifierGlowImage	amplifier(size, center, 6.5e-6, 1, 0.002);
	ImagePtr	image(new Image<double>(amplifier));
	io::FITSout	out("tmp/amplifier.fits");
	out.setPrecious("false");
	out.write(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAmplifier() end");
}

#if 0
void	AdapterTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
