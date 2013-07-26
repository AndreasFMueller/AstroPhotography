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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCopy() begin");
	Image<RGB<unsigned char> >	src(16, 9);
	for (unsigned int x = 0; x < src.size.getWidth(); x++) {
		for (unsigned int y = 0; y < src.size.getHeight(); y++) {
			src.pixel(x, y).R = x << 4;
			src.pixel(x, y).G = y << 4;
			src.pixel(x, y).B = x + y;
		}
	}
	Image<RGB<unsigned char> >	dst(src);
	for (unsigned int x = 0; x < src.size.getWidth(); x++) {
		for (unsigned int y = 0; y < src.size.getHeight(); y++) {
			CPPUNIT_ASSERT(dst.pixel(x, y).R == x << 4);
			CPPUNIT_ASSERT(dst.pixel(x, y).G == y << 4);
			CPPUNIT_ASSERT(dst.pixel(x, y).B == x + y);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCopy() end");
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRgb() begin");
	Image<YUYV<unsigned char> >	*src = new Image<YUYV<unsigned char> >(16, 9);
	for (unsigned int x = 0; x < src->size.getWidth(); x++) {
		for (unsigned int y = 0; y < src->size.getHeight(); y++) {
			src->pixel(x, y).y = x << 4;
			src->pixel(x, y).uv = y << 4;
		}
	}
	RGBImagePtr	rgb(new Image<RGB<unsigned char> >(*src));
	for (unsigned int x = 0; x < src->size.getWidth(); x += 2) {
		for (unsigned int y = 0; y < src->size.getHeight(); y++) {
			RGB<unsigned char>	man;
			int	d = src->pixel(x, y).uv - 128;
			int	e = src->pixel(x + 1, y).uv - 128;
			int	c = src->pixel(x, y).y - 16;
			man.R = red(c, d, e);
			man.G = green(c, d, e);
			man.B = blue(c, d, e);
#if 0
std::cerr << "R = " << (int)rgb->pixel(x, y).R
	<< ", G = " << (int)rgb->pixel(x, y).G
	<< ", B = " << (int)rgb->pixel(x, y).B << std::endl;
std::cerr << "man.R = " << (int)man.R
	<< ", man.G = " << (int)man.G
	<< ", man.B = " << (int)man.B << std::endl;
#endif
			CPPUNIT_ASSERT(rgb->pixel(x, y) == man);
			c = src->pixel(x + 1, y).y - 16;
			man.R = red(c, d, e);
			man.G = green(c, d, e);
			man.B = blue(c, d, e);
			CPPUNIT_ASSERT(rgb->pixel(x + 1, y) == man);
			
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRgb() end");
}

} // namespace test
} // namespace astro
