/*
 * StellariumTest.cpp -- test the outline stuff
 *
 *  (c) 2018 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCatalog.h>
#include <AstroDebug.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include "../Stellarium.h"

namespace astro {
namespace test {

class StellariumTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testStellarium();

	CPPUNIT_TEST_SUITE(StellariumTest);
	CPPUNIT_TEST(testStellarium);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(StellariumTest);

void	StellariumTest::setUp() {
}

void	StellariumTest::tearDown() {
}

void	StellariumTest::testStellarium() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testStellarium() begin");
	catalog::Stellarium	sc;
	//CPPUNIT_ASSERT(fabs(C.x()) < 1e-8);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testStellarium() end");
}

} // namespace test
} // namespace astro
