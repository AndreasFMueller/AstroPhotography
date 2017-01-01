/*
 * MedianTest.cpp -- template for tests
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroUtils.h>

namespace astro {
namespace test {

class MedianTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testMedian();
	void	testEmpty();

	CPPUNIT_TEST_SUITE(MedianTest);
	CPPUNIT_TEST(testMedian);
	CPPUNIT_TEST_EXCEPTION(testEmpty, std::range_error);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(MedianTest);

void	MedianTest::setUp() {
}

void	MedianTest::tearDown() {
}

void	MedianTest::testEmpty() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testEmpty() begin");
	Median<unsigned char>	m;
	m.median();
}

void	MedianTest::testMedian() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMedian() begin");
	Median<unsigned char>	m;
	m.add(12);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "median: %d", (int)m.median());
	CPPUNIT_ASSERT(m.median() == 12);
	m.add(14);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "median: %d", (int)m.median());
	CPPUNIT_ASSERT(m.median() == 13);
	m.add(16);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "median: %d", (int)m.median());
	CPPUNIT_ASSERT(m.median() == 14);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMedian() end");
}

} // namespace test
} // namespace astro
