/*
 * TestTemplate.cpp -- template for tests
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <FieldPersistence.h>

using namespace astro::persistence;

namespace astro {
namespace test {

class TestTemplate: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testTime1();
	void	testTime2();
	void	testTime3();
	void	testTime4();

	CPPUNIT_TEST_SUITE(TestTemplate);
	CPPUNIT_TEST(testTime1);
	CPPUNIT_TEST(testTime2);
	CPPUNIT_TEST(testTime3);
	CPPUNIT_TEST(testTime4);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestTemplate);

void	TestTemplate::setUp() {
}

void	TestTemplate::tearDown() {
}

void	TestTemplate::testTime1() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testTime1() begin");
	time_t	t = 1414611916;
	std::string	time = TimeField::time2string(t);
	CPPUNIT_ASSERT(time == std::string("2014-10-29 20:45:16"));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testTime1() end");
}

void	TestTemplate::testTime2() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testTime2() begin");
	std::string	time("2014-10-29 20:45:16");
	time_t	t = TimeField::string2time(time);
	CPPUNIT_ASSERT(t == 1414611916);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testTime2() end");
}

void	TestTemplate::testTime3() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testTime3() begin");
	time_t	t = 1414611916;
	std::string	time("2014-10-29 20:45:16");
	std::string	time2 = TimeField::time2string(t);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "'%s' ?= '%s' %d", time.c_str(), time2.c_str(), t);
	CPPUNIT_ASSERT(time == time2);
	for (int i = 0; i < 100; i++) {
		t -= 54321;
		std::string	time1 = TimeField::time2string(t);
		int	t2 = TimeField::string2time(time1);
		std::string	time2 = TimeField::time2string(t2);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s ?= %s, %d ?= %d, %d",
			time1.c_str(), time2.c_str(), t, t2, t - t2);
		CPPUNIT_ASSERT(time1 == time2);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testTime3() end");
}

void	TestTemplate::testTime4() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testTime4() begin");
	time_t	t = 1414611916;
	std::string	time("2014-10-29 20:45:16");
	std::string	time2 = TimeField::time2string(t);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "'%s' ?= '%s' %d", time.c_str(), time2.c_str(), t);
	CPPUNIT_ASSERT(time == time2);
	for (int i = 0; i < 100; i++) {
		t -= 54321;
		std::string	time1 = TimeField::time2string(t);
		int	t2 = TimeField::string2time(time1);
		std::string	time2 = TimeField::time2string(t2);
		int	t3 = TimeField::string2time(time2);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s ?= %s, %d ?= %d, %d",
			time1.c_str(), time2.c_str(), t2, t3, t3 - t2);
		CPPUNIT_ASSERT(t2 == t3);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testTime4() end");
}

#if 0
void	TestTemplate::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
