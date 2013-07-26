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
#include <AstroDebug.h>

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
	//CPPUNIT_TEST(testMedianLarge);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(FilterTest);

void	FilterTest::setUp() {
}

void	FilterTest::tearDown() {
}

void	FilterTest::testMin() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMin() begin");
	Image<unsigned char>	image(10, 10);
	for (unsigned int x = 0; x < image.size.getWidth(); x++) {
		for (unsigned int y = 0; y < image.size.getHeight(); y++) {
			image.pixel(x, y) = 7 + x + y;
		}
	}
	Min<unsigned char, unsigned char>	m;
	CPPUNIT_ASSERT(7 == m(image));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMin() end");
}

void	FilterTest::testMax() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMax() begin");
	Image<unsigned char>	image(10, 10);
	for (unsigned int x = 0; x < image.size.getWidth(); x++) {
		for (unsigned int y = 0; y < image.size.getHeight(); y++) {
			image.pixel(x, y) = 7 + x + y;
		}
	}
	Max<unsigned char, unsigned char>	m;
	CPPUNIT_ASSERT(25 == m(image));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMax() end");
}

void	FilterTest::testMean() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMean() begin");
	Image<unsigned char>	image(10, 10);
	for (unsigned int x = 0; x < image.size.getWidth(); x++) {
		for (unsigned int y = 0; y < image.size.getHeight(); y++) {
			image.pixel(x, y) = 7 + x + y;
		}
	}
	Mean<unsigned char, double>	m;
	CPPUNIT_ASSERT(16 == m(image));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMean() end");
}

void	FilterTest::testMedian() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMedian() begin");
	Image<unsigned short>	image(100, 100);
	for (unsigned int x = 0; x < image.size.getWidth(); x++) {
		for (unsigned int y = 0; y < image.size.getHeight(); y++) {
			image.pixel(x, y) = 7 * x * y;
		}
	}
	Median<unsigned short, unsigned short>	m;
	CPPUNIT_ASSERT(12641 == m(image));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMedian() end");
}

void	FilterTest::testMedian0() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMedian0() begin");
	Image<unsigned short>	image(100, 100);
	for (unsigned int x = 0; x < image.size.getWidth(); x++) {
		for (unsigned int y = 0; y < image.size.getHeight(); y++) {
			image.pixel(x, y) = 0;
		}
	}
	Median<unsigned short, unsigned short>	m;
	unsigned short	median = m(image);
	CPPUNIT_ASSERT(0 == median);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMedian0() end");
}

void	FilterTest::testMedianLarge() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMedianLarge() begin");
	Image<unsigned int>	image(8000, 8000);
	for (unsigned int x = 0; x < image.size.getWidth(); x++) {
		for (unsigned int y = 0; y < image.size.getHeight(); y++) {
			image.pixel(x, y) = x * y;
		}
	}
	Median<unsigned int, unsigned int>	m;
	unsigned int	median = m(image);
	CPPUNIT_ASSERT(11943792 == median);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMedianLarge() end");
}

} // namespace test
} // namespace astro
