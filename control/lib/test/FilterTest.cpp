
/*
 * FilterTest.cpp -- test pixel conversions
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroImage.h>
#include <AstroFilter.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

using namespace astro::image;
using namespace astro::image::filter;

namespace astro {
namespace test {

class FilterTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testMin();
	void	testMax();
	void	testMean();
	void	testMedian();
	void	testMedian0();
	void	testMedianLarge();

	CPPUNIT_TEST_SUITE(FilterTest);
	CPPUNIT_TEST(testMin);
	CPPUNIT_TEST(testMax);
	CPPUNIT_TEST(testMean);
	CPPUNIT_TEST(testMedian);
	CPPUNIT_TEST(testMedian0);
	CPPUNIT_TEST(testMedianLarge);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(FilterTest);

void	FilterTest::setUp() {
}

void	FilterTest::tearDown() {
}

void	FilterTest::testMin() {
	Image<unsigned char>	image(10, 10);
	for (unsigned int x = 0; x < image.size.width; x++) {
		for (unsigned int y = 0; y < image.size.height; y++) {
			image.pixel(x, y) = 7 + x + y;
		}
	}
	Min<unsigned char>	m;
	CPPUNIT_ASSERT(7 == m(image));
}

void	FilterTest::testMax() {
	Image<unsigned char>	image(10, 10);
	for (unsigned int x = 0; x < image.size.width; x++) {
		for (unsigned int y = 0; y < image.size.height; y++) {
			image.pixel(x, y) = 7 + x + y;
		}
	}
	Max<unsigned char>	m;
	CPPUNIT_ASSERT(25 == m(image));
}

void	FilterTest::testMean() {
	Image<unsigned char>	image(10, 10);
	for (unsigned int x = 0; x < image.size.width; x++) {
		for (unsigned int y = 0; y < image.size.height; y++) {
			image.pixel(x, y) = 7 + x + y;
		}
	}
	Mean<unsigned char, double>	m;
	CPPUNIT_ASSERT(16 == m(image));
}

void	FilterTest::testMedian() {
	Image<unsigned short>	image(100, 100);
	for (unsigned int x = 0; x < image.size.width; x++) {
		for (unsigned int y = 0; y < image.size.height; y++) {
			image.pixel(x, y) = 7 * x * y;
		}
	}
	Median<unsigned short>	m;
	CPPUNIT_ASSERT(12641 == m(image));
}

void	FilterTest::testMedian0() {
	Image<unsigned short>	image(100, 100);
	for (unsigned int x = 0; x < image.size.width; x++) {
		for (unsigned int y = 0; y < image.size.height; y++) {
			image.pixel(x, y) = 0;
		}
	}
	Median<unsigned short>	m;
	unsigned short	median = m(image);
	CPPUNIT_ASSERT(0 == median);
}

void	FilterTest::testMedianLarge() {
	Image<unsigned int>	image(8000, 8000);
	for (unsigned int x = 0; x < image.size.width; x++) {
		for (unsigned int y = 0; y < image.size.height; y++) {
			image.pixel(x, y) = x * y;
		}
	}
	Median<unsigned int>	m;
	unsigned int	median = m(image);
	CPPUNIT_ASSERT(11943792 == median);
}

} // namespace test
} // namespace astro
