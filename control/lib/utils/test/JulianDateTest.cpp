/*
 * JulianDateTest.cpp -- template for tests
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroCoordinates.h>

namespace astro {
namespace test {

class JulianDateTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testJD();
	void	testJD2();
	void	testGMST();
	void	testGMST2();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(JulianDateTest);
	CPPUNIT_TEST(testJD);
	CPPUNIT_TEST(testJD2);
	CPPUNIT_TEST(testGMST);
	CPPUNIT_TEST(testGMST2);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(JulianDateTest);

void	JulianDateTest::setUp() {
}

void	JulianDateTest::tearDown() {
}

void	JulianDateTest::testJD() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testJD() begin");
	struct tm	when;
	memset(&when, 0, sizeof(when));
	when.tm_year = 100;
	when.tm_mon = 0;
	when.tm_mday = 1;
	when.tm_hour = 12;
	when.tm_min = 0;
	when.tm_sec = 0;
	time_t	t = timegm(&when);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "time = %ld", t);
	JulianDate	jd(t);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "T = %.4f", jd.T());
	CPPUNIT_ASSERT(jd.T() == 2451545.0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testJD() end");
}

void	JulianDateTest::testJD2() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testJD2() begin");
	struct tm	when;
	memset(&when, 0, sizeof(when));
	when.tm_year = 106;
	when.tm_mon = 0;
	when.tm_mday = 14;
	when.tm_hour = 16;
	when.tm_min = 30;
	when.tm_sec = 0;
	time_t	t = timegm(&when);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "time = %ld", t);
	JulianDate	jd(t);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "T = %.4f", jd.T());
	CPPUNIT_ASSERT(fabs(jd.T() - 2453750.1875) < 0.00005);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testJD2() end");
}

void	JulianDateTest::testGMST() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testGMST() begin");
	struct tm	when;
	memset(&when, 0, sizeof(when));
	when.tm_year = 119;
	when.tm_mon = 0;
	when.tm_mday = 1;
	when.tm_hour = 8;
	when.tm_min = 0;
	when.tm_sec = 0;
	time_t	t = timegm(&when);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "time = %ld", t);
	JulianDate	jd(t);
	Angle	gmst = jd.GMST();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "GMST = %s", gmst.hms().c_str());
	CPPUNIT_ASSERT(gmst.hms().substr(1,8) == std::string("14:42:45"));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testGMST() end");
}

void	JulianDateTest::testGMST2() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testGMST() begin");
	struct tm	when;
	memset(&when, 0, sizeof(when));
	when.tm_year = 118;
	when.tm_mon = 1;
	when.tm_mday = 14;
	when.tm_hour = 14;
	when.tm_min = 0;
	when.tm_sec = 0;
	time_t	t = timegm(&when);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "time = %ld", t);
	JulianDate	jd(t);
	Angle	gmst = jd.GMST();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "GMST = %s", gmst.hms().c_str());
	CPPUNIT_ASSERT(gmst.hms().substr(1,8) == std::string("23:38:10"));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testGMST2() end");
}

#if 0
void	JulianDateTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
