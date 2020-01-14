/*
 * MoonTest.cpp -- template for tests
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroCoordinates.h>
#include <AstroSolarsystem.h>

namespace astro {
namespace test {

class MoonTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testMoon();
	void	testYear();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(MoonTest);
	CPPUNIT_TEST(testMoon);
	CPPUNIT_TEST(testYear);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(MoonTest);

void	MoonTest::setUp() {
}

void	MoonTest::tearDown() {
}

void	MoonTest::testMoon() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMoon() begin");
	solarsystem::Moon	moon;
	time_t	now;
	time(&now);
	RaDec	where = moon.ephemeris(now);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Moon: %s", where.toString().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMoon() end");
}

/*

Mond
Datum      RA  (J2000)  Dec
                           
2020     h  m   s    o  '  "
Jan  1  23:11:21.7 -10:52:39
Feb  1   1:52:14.4  +5:55:18
Mär  3   4:48:55.0 +20:01:23
Apr  3   8:16:19.8 +21:41:18
Mai  4  11:45:14.8  +6:22:58
Jun  4  15:05:52.0 -14:45:39
Jul  5  18:42:44.4 -24:58:20
Aug  5  22:06:18.1 -17:18:52
Sep  5   0:55:28.5  -0:07:20
Okt  6   3:42:04.7 +16:46:04
Nov  6   6:58:32.1 +24:19:15
Dez  7  10:24:18.9 +14:57:36

 */

void	MoonTest::testYear() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testYear() begin");
	solarsystem::Moon	moon;
	struct tm	stm;
	stm.tm_sec = 0;
	stm.tm_min = 0;
	stm.tm_hour = 0;
	stm.tm_mday = 1;
	stm.tm_mon = 0;
	stm.tm_year = 120;
	stm.tm_isdst = 0;
	stm.tm_zone = NULL;
	stm.tm_gmtoff = -3600;
	time_t	now = timelocal(&stm);
	for (int t = 0; t < 12; t++) {
		RaDec	where = moon.ephemeris(now + t * 31 * 86400);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "Moon: %s",
			where.toString().c_str());
	}
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testYear() end");
}

#if 0
void	MoonTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
