/*
 * SunTest.cpp -- template for tests
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

class SunTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testSun();
	void	testYear();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(SunTest);
	CPPUNIT_TEST(testSun);
	CPPUNIT_TEST(testYear);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SunTest);

void	SunTest::setUp() {
}

void	SunTest::tearDown() {
}

void	SunTest::testSun() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSun() begin");
	solarsystem::Sun	sun;
	time_t	now;
	time(&now);
	RaDec	where = sun.ephemeris(now);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Sun: %s", where.toString().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSun() end");
}

/*
 
Sonne
Datum      RA  (J2000)  Dec  
                             
2020     h  m   s    o  '  "
Jan  1  18:42:10.7 -23:05:02
Feb  1  20:54:50.5 -17:23:59
Mär  3  22:55:39.6  -6:51:01
Apr  3   0:49:05.9  +5:15:50
Mai  4   2:44:43.3 +15:55:42
Jun  4   4:48:21.7 +22:24:49
Jul  5   6:56:50.4 +22:47:30
Aug  5   9:00:33.9 +17:00:00
Sep  5  10:55:34.0  +6:51:22
Okt  6  12:47:12.0  -5:04:04
Nov  6  14:44:58.7 -15:57:03
Dez  7  16:54:53.2 -22:35:38

 */

void	SunTest::testYear() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testYear() begin");
	solarsystem::Sun	sun;
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
		RaDec	where = sun.ephemeris(now + t * 31 * 86400);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "Sun: %s",
			where.toString().c_str());
	}
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testYear() end");
}

#if 0
void	SunTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
