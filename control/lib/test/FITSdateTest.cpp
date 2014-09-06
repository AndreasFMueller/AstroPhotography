/*
 * FITSdateTest.cpp -- template for tests
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroImage.h>
#include <includes.h>

using namespace astro::image;

namespace astro {
namespace test {

class FITSdateTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testShort();
	void	testLong();
	void	testVeryLong();
	void	testComparison();
	void	testCast();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(FITSdateTest);
	CPPUNIT_TEST(testShort);
	CPPUNIT_TEST(testLong);
	CPPUNIT_TEST(testVeryLong);
	CPPUNIT_TEST(testComparison);
	CPPUNIT_TEST(testCast);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(FITSdateTest);

void	FITSdateTest::setUp() {
}

void	FITSdateTest::tearDown() {
}

void	FITSdateTest::testShort() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() begin");

	std::string	s("1962-02-14");
	FITSdate	d(s);
	CPPUNIT_ASSERT(d.showShort() == s);
	CPPUNIT_ASSERT(d.showLong() == "1962-02-14T00:00:00");
	CPPUNIT_ASSERT(d.showVeryLong() == "1962-02-14T00:00:00.000");

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() end");
}

void	FITSdateTest::testLong() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() begin");

	std::string	s("1999-04-18T12:13:14");
	FITSdate	d(s);
	CPPUNIT_ASSERT(d.showLong() == s);
	CPPUNIT_ASSERT(d.showShort() == "1999-04-18");
	CPPUNIT_ASSERT(d.showVeryLong() == "1999-04-18T12:13:14.000");

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() end");
}

void	FITSdateTest::testVeryLong() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() begin");

	std::string	s("2001-12-31T21:32:43.000");
	FITSdate	d(s);
	CPPUNIT_ASSERT(d.showVeryLong() == s);
	CPPUNIT_ASSERT(d.showLong() == "2001-12-31T21:32:43");
	CPPUNIT_ASSERT(d.showShort() == "2001-12-31");

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() end");
}

void	FITSdateTest::testComparison() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testComparison() begin");
	FITSdate	past("1962-02-14");
	FITSdate	now;
	CPPUNIT_ASSERT(past < now);
	CPPUNIT_ASSERT(past == past);
	CPPUNIT_ASSERT(now == now);
	CPPUNIT_ASSERT(!(past == now));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testComparison() end");
}

void	FITSdateTest::testCast() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCast() begin");
	struct timeval	tv;
	gettimeofday(&tv, NULL);
	FITSdate	now(tv);
	CPPUNIT_ASSERT((std::string)now == now.showVeryLong());
	struct timeval	tv2 = (struct timeval)now;
	CPPUNIT_ASSERT(tv2.tv_sec == tv.tv_sec);
	CPPUNIT_ASSERT(tv2.tv_usec == tv.tv_usec);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCast() end");
}

#if 0
void	FITSdateTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
