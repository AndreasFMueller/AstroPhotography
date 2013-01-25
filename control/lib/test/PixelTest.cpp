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

	CPPUNIT_TEST_SUITE(PixelTest);
	CPPUNIT_TEST(testMonochrome);
	CPPUNIT_TEST(testMono2RGB);
	CPPUNIT_TEST(testRGB2Mono);
	CPPUNIT_TEST(testRGB2YUYV);
	CPPUNIT_TEST(testYUYV2RGB);
	CPPUNIT_TEST(testRGB2YUYVfloat);
	CPPUNIT_TEST(testYUYV2RGBfloat);
	CPPUNIT_TEST(testConversionParameters);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(PixelTest);

void	PixelTest::setUp() {
}

void	PixelTest::tearDown() {
}

void	PixelTest::testMonochrome() {
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
}

void	PixelTest::testMono2RGB() {
	RGB<unsigned short>	r;
	unsigned int	ui = 0x12345678;
	unsigned int	ui2;
	convertPixel(ui2, ui);
	CPPUNIT_ASSERT(ui == ui2);
	convertPixel(r, ui);
	CPPUNIT_ASSERT(r.R == 0x1234);
	CPPUNIT_ASSERT(r.G == 0x1234);
	CPPUNIT_ASSERT(r.B == 0x1234);
}

void	PixelTest::testRGB2Mono() {
	RGB<unsigned short>	r;
	r.R = 4711;
	r.G = 4711;
	r.B = 4711;
	unsigned short	v;
	convertPixel(v, r);
	CPPUNIT_ASSERT(v == 4711);
}

void	PixelTest::testYUYV2RGB() {
	RGB<unsigned short>	r[2];
	YUYV<unsigned short>	y[2];
	convertPixelPair(y, r);
	convertPixelArray(y, r, 2);
}

void	PixelTest::testRGB2YUYV() {
	YUYV<unsigned short>	y[2];
	RGB<unsigned short>	r[2];
	convertPixelPair(r, y);
	convertPixelArray(r, y, 2);
}

void	PixelTest::testYUYV2RGBfloat() {
	RGB<float>	r[2];
	YUYV<float>	y[2];
	convertPixelPair(y, r);
	convertPixelArray(y, r, 2);
}

void	PixelTest::testRGB2YUYVfloat() {
	YUYV<float>	y[2];
	RGB<float>	r[2];
	convertPixelPair(r, y);
	convertPixelArray(r, y, 2);
}

void	PixelTest::testConversionParameters() {
#if 0
#define	SHOW_CONSTANTS(T)					\
	std::cerr << "size " << sizeof(T) 			\
		<< ": pedestal = "				\
		<< (unsigned long)Color<T>::pedestal		\
		<< ", zero = "					\
		<< (unsigned long)Color<T>::zero		\
		<< ", limit = " 				\
		<< (double)Color<T>::limit		\
		<< std::endl;
	SHOW_CONSTANTS(unsigned char)
	SHOW_CONSTANTS(unsigned short)
	SHOW_CONSTANTS(unsigned int)
	SHOW_CONSTANTS(unsigned long)
	SHOW_CONSTANTS(float)
	SHOW_CONSTANTS(double)
#endif

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
}

} // namespace test
} // namespace astro
