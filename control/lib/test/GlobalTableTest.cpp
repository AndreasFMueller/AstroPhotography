/*
 * GlobalTableTest.cpp -- Test the global configuraiton variables table
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <includes.h>
#include <GlobalTable.h>

using namespace astro::persistence;
using namespace astro::config;

namespace astro {
namespace test {

class GlobalTableTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testGlobal();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(GlobalTableTest);
	CPPUNIT_TEST(testGlobal);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(GlobalTableTest);

void	GlobalTableTest::setUp() {
}

void	GlobalTableTest::tearDown() {
}

void	GlobalTableTest::testGlobal() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testGlobal() begin");
	unlink("globaltest.db");
	Database	database = DatabaseFactory::get("globaltest.db");
	GlobalTable	globals(database);
	GlobalRecord	global1;
	global1.name = "global1";
	global1.section = "Description of global1";
	global1.value = stringprintf("%d", time(NULL) - 86400);
	long	id1 = globals.add(global1);
	GlobalRecord	global2;
	global2.name = "global2";
	global2.section = "Description of global2";
	global2.value = stringprintf("%d", time(NULL) - 10 * 86400);
	long	id2 = globals.add(global2);
	GlobalRecord	global3 = globals.byid(id1);
	CPPUNIT_ASSERT(global1.name == global3.name);
	CPPUNIT_ASSERT(global1.section == global3.section);
	CPPUNIT_ASSERT(global1.value == global3.value);
	GlobalRecord	global4 = globals.byid(id2);
	CPPUNIT_ASSERT(global2.name == global4.name);
	CPPUNIT_ASSERT(global2.section == global4.section);
	CPPUNIT_ASSERT(global2.value == global4.value);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testGlobal() end");
}

#if 0
void	GlobalTableTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
