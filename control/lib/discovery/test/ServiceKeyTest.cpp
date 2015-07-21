/*
 * ServiceKeyTest.cpp -- test the ServiceKey class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <ServiceDiscovery.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

namespace astro {
namespace test {

class ServiceKeyTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testConstructor();
	void	testCompare();

	CPPUNIT_TEST_SUITE(ServiceKeyTest);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testCompare);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ServiceKeyTest);

void	ServiceKeyTest::setUp() {
}

void	ServiceKeyTest::tearDown() {
}

void	ServiceKeyTest::testConstructor() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() begin");
	discover::ServiceKey	key("name1", "_astro._tcp", "othello.local");
	CPPUNIT_ASSERT(key.name() == "name1");
	CPPUNIT_ASSERT(key.type() == "_astro._tcp");
	CPPUNIT_ASSERT(key.domain() == "othello.local");
	key.interface(3);
	CPPUNIT_ASSERT(key.interface() == 3);
	key.protocol(47);
	CPPUNIT_ASSERT(key.protocol() == 47);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() end");
}

void	ServiceKeyTest::testCompare() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCompare() begin");
	discover::ServiceKey	key1("name1", "_astro._tcp", "othello.local");
	CPPUNIT_ASSERT(key1 == key1);
	discover::ServiceKey	key2("name2", "_astro._tcp", "othello.local");
	CPPUNIT_ASSERT(!(key1 == key2));
	CPPUNIT_ASSERT(key1 < key2);
	CPPUNIT_ASSERT(!(key1 < key1));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCompare() end");
}

} // namespace test
} // namespace astro
