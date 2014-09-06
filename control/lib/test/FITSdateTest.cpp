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
	//void	testXXX();

	CPPUNIT_TEST_SUITE(FITSdateTest);
	CPPUNIT_TEST(testShort);
	CPPUNIT_TEST(testLong);
	CPPUNIT_TEST(testVeryLong);
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

#if 0
void	FITSdateTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
