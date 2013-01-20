/*
 * RGBTest.cpp -- tests for the RGB Pixel stuff
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil 
 * $Id$
 */
#include <AstroImage.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

using namespace astro::image;

namespace astro {
namespace test {

class RGBTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testCopy();
	void	testRgb();

	CPPUNIT_TEST_SUITE(RGBTest);
	CPPUNIT_TEST(testCopy);
	CPPUNIT_TEST(testRgb);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(RGBTest);

void	RGBTest::setUp() {
}

void	RGBTest::tearDown() {
}

void	RGBTest::testCopy() {
	Image<RGBPixel>	src(16, 9);
	for (int x = 0; x < src.size.width; x++) {
		for (int y = 0; y < src.size.height; y++) {
			src.pixel(x, y).R = x << 4;
			src.pixel(x, y).G = y << 4;
			src.pixel(x, y).B = x + y;
		}
	}
	Image<RGBPixel>	dst(src);
	for (int x = 0; x < src.size.width; x++) {
		for (int y = 0; y < src.size.height; y++) {
			CPPUNIT_ASSERT(dst.pixel(x, y).R == x << 4);
			CPPUNIT_ASSERT(dst.pixel(x, y).G == y << 4);
			CPPUNIT_ASSERT(dst.pixel(x, y).B == x + y);
		}
	}
}

static unsigned char    limit(int x) {
        if (x > 255) { return 255; }
        if (x < 0) { return 0; }
        unsigned char   result = 0xff & x;
        return result;
}

/*
 * YUYV conversion formulas based on
 * http://msdn.microsoft.com/en-us/library/windows/desktop/dd206750(v=vs.85).aspx
 */

static unsigned char    red(int c, int d, int e) {
        return limit((298 * c           + 409 * e + 128) >> 8);
}
static unsigned char    green(int c, int d, int e) {
        return limit((298 * c - 100 * d - 208 * e + 128) >> 8);
}
static unsigned char    blue(int c, int d, int e) {
        return limit((298 * c + 516 * d           + 128) >> 8);
}


void	RGBTest::testRgb() {
	Image<YUYVPixel>	*src = new Image<YUYVPixel>(16, 9);
	for (int x = 0; x < src->size.width; x++) {
		for (int y = 0; y < src->size.height; y++) {
			src->pixel(x, y).y = x << 4;
			src->pixel(x, y).uv = y << 4;
		}
	}
	YuyvImage	image(src);
	RgbImage	rgb = imageConvert<RGBPixel, YUYVPixel>(image);
	for (int x = 0; x < src->size.width; x += 2) {
		for (int y = 0; y < src->size.height; y++) {
			RGBPixel	man;
			int	d = src->pixel(x, y).uv - 128;
			int	e = src->pixel(x + 1, y).uv - 128;
			int	c = src->pixel(x, y).y - 16;
			man.R = red(c, d, e);
			man.G = green(c, d, e);
			man.B = blue(c, d, e);
			CPPUNIT_ASSERT(rgb->pixel(x, y) == man);
			c = src->pixel(x + 1, y).y - 16;
			man.R = red(c, d, e);
			man.G = green(c, d, e);
			man.B = blue(c, d, e);
			CPPUNIT_ASSERT(rgb->pixel(x + 1, y) == man);
			
		}
	}
	
}

} // namespace test
} // namespace astro
