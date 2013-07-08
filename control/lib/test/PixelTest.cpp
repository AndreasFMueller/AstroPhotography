/*
 * PixelTest.cpp -- test pixel conversions
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroImage.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <debug.h>

using namespace astro::image;

namespace astro {
namespace test {

class PixelTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testMonochrome();
	void	testMono2RGB();
	void	testRGB2Mono();
	void	testRGB2YUYV();
	void	testYUYV2RGB();
	void	testRGB2YUYVfloat();
	void	testYUYV2RGBfloat();
	void	testConversionParameters();
	void	testCharAndShort();

	CPPUNIT_TEST_SUITE(PixelTest);
	CPPUNIT_TEST(testMonochrome);
	CPPUNIT_TEST(testMono2RGB);
	CPPUNIT_TEST(testRGB2Mono);
	CPPUNIT_TEST(testRGB2YUYV);
	CPPUNIT_TEST(testYUYV2RGB);
	CPPUNIT_TEST(testRGB2YUYVfloat);
	CPPUNIT_TEST(testYUYV2RGBfloat);
	CPPUNIT_TEST(testConversionParameters);
	CPPUNIT_TEST(testCharAndShort);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(PixelTest);

void	PixelTest::setUp() {
}

void	PixelTest::tearDown() {
}

void	PixelTest::testMonochrome() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMonochrome() begin");
	unsigned short	us = 65535;
	unsigned char	uc;
	convertPixelValue(uc, us);
	CPPUNIT_ASSERT(uc == 255);
	convertPixelValue(us, uc);
	CPPUNIT_ASSERT(us == 0xff00);

	unsigned long	ul = 0x1234567812345678;
	unsigned int	ui;
	convertPixelValue(ui, ul);
	convertPixelValue(ul, ui);
	CPPUNIT_ASSERT(ul = 0x1234567800000000);

	convertPixelValue(us, ul);
	CPPUNIT_ASSERT(us == 0x1234);
	convertPixelValue(ul, us);
	CPPUNIT_ASSERT(ul == 0x1234000000000000);

	float	f = 255.;
	convertPixelValue(uc, f);
	CPPUNIT_ASSERT(uc == 255);

	// float 257 overflows the result...
	f = 257.;
	convertPixelValue(uc, f);
	CPPUNIT_ASSERT(uc == 1);

	long long	ll = 0x1122334455667788;
	convertPixelValue(ui, ll);
	convertPixelValue(ll, ui);
	CPPUNIT_ASSERT(0x1122334400000000 == ll);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMonochrome() end");
}

void	PixelTest::testMono2RGB() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMono2RGB() begin");
	RGB<unsigned short>	r;
	unsigned int	ui = 0x12345678;
	unsigned int	ui2;
	convertPixel(ui2, ui);
	CPPUNIT_ASSERT(ui == ui2);
	convertPixel(r, ui);
	CPPUNIT_ASSERT(r.R == 0x1234);
	CPPUNIT_ASSERT(r.G == 0x1234);
	CPPUNIT_ASSERT(r.B == 0x1234);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMono2RGB() end");
}

void	PixelTest::testRGB2Mono() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRGB2Mono() begin");
	RGB<unsigned short>	r;
	r.R = 4711;
	r.G = 4711;
	r.B = 4711;
	unsigned short	v;
	convertPixel(v, r);
	CPPUNIT_ASSERT(v == 4711);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRGB2Mono() end");
}

void	PixelTest::testYUYV2RGB() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testYUYV2RGB() begin");
	RGB<unsigned short>	r[2];
	YUYV<unsigned short>	y[2];
	convertPixelPair(y, r);
	convertPixelArray(y, r, 2);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testYUYV2RGB() end");
}

void	PixelTest::testRGB2YUYV() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRGB2YUYV() begin");
	YUYV<unsigned short>	y[2];
	RGB<unsigned short>	r[2];
	convertPixelPair(r, y);
	convertPixelArray(r, y, 2);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRGB2YUYV() end");
}

void	PixelTest::testYUYV2RGBfloat() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testYUYV2RGBfloat() begin");
	RGB<float>	r[2];
	YUYV<float>	y[2];
	convertPixelPair(y, r);
	convertPixelArray(y, r, 2);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testYUYV2RGBfloat() end");
}

void	PixelTest::testRGB2YUYVfloat() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRGB2YUYVfloat() begin");
	YUYV<float>	y[2];
	RGB<float>	r[2];
	convertPixelPair(r, y);
	convertPixelArray(r, y, 2);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRGB2YUYVfloat() end");
}

void	PixelTest::testCharAndShort() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCharAndShort() begin");
	unsigned char	c1 = 16;
	unsigned short	s1;
	convertPixel(s1, c1);
	CPPUNIT_ASSERT(s1 == (16 * 256));
	convertPixel(c1, s1);
	CPPUNIT_ASSERT(c1 == 16);
	RGB<unsigned char>	c((unsigned char)5, (unsigned char)10, (unsigned char)15);
	RGB<unsigned short>	s;
	convertPixel(s, c);
	unsigned char	c2;
	convertPixel(c2, c);
	unsigned char	s2;
	convertPixel(s2, s);
	unsigned char	c3;
	convertPixel(c3, s2);
	CPPUNIT_ASSERT(c3 == c2);
	//std::cerr << "c2 = " << (int)c2 << ", c3 = " << (int)c3 << std::endl;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCharAndShort() end");
}

void	PixelTest::testConversionParameters() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConversionParameters() begin");

#define	COLOR_ASSERTS(T, P)						\
	CPPUNIT_ASSERT(Color<T>::pedestal == (((P)16) << ((sizeof(P) - 1) << 3))); \
	CPPUNIT_ASSERT(Color<T>::zero == (((P)128) << ((sizeof(P) - 1) << 3)));\
	CPPUNIT_ASSERT(Color<T>::limit == (P)-1);

	COLOR_ASSERTS(unsigned char, unsigned char);
	COLOR_ASSERTS(unsigned short, unsigned short);
	COLOR_ASSERTS(unsigned int, unsigned int);
	COLOR_ASSERTS(unsigned long, unsigned long);
	COLOR_ASSERTS(float, unsigned int);
	COLOR_ASSERTS(double, unsigned long);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConversionParameters() end");
}

} // namespace test
} // namespace astro
