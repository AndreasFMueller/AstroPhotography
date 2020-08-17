/*
 * PropertiesTest.cpp -- test the device name class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroDevice.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <stdlib.h>

namespace astro {
namespace test {

class PropertiesTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testSet();

	CPPUNIT_TEST_SUITE(PropertiesTest);
	CPPUNIT_TEST(testSet);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(PropertiesTest);

void	PropertiesTest::setUp() {
}

void	PropertiesTest::tearDown() {
}

void	PropertiesTest::testSet() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSet() begin");
	putenv(strdup("DEVICEPROPERTIES=test.properties"));
	astro::Properties	properties("ccd:sx/001-137/Imaging");
	CPPUNIT_ASSERT(properties.getProperty("limit") == "4711");
	CPPUNIT_ASSERT(properties.getProperty("min") == "klein");
	CPPUNIT_ASSERT(properties.getProperty("max") == "gross");
	unsetenv("DEVICEPROPERTIES");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSet() end");
}

} // namespace test
} // namespace astro
