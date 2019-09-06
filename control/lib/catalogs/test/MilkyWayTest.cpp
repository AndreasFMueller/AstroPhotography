/*
 * MilkyWayTest.cpp -- tests for the MilkyWay catalog 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCatalog.h>
#include <AstroDebug.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

using namespace astro::catalog;

namespace astro {
namespace test {

class MilkyWayTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testConstructor();

	CPPUNIT_TEST_SUITE(MilkyWayTest);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(MilkyWayTest);

void	MilkyWayTest::setUp() {
}

void	MilkyWayTest::tearDown() {
}

void	MilkyWayTest::testConstructor() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() begin");
	MilkyWayPtr	milkyway = MilkyWay::get();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() end");
}

} // namespace test
} // namespace astro
