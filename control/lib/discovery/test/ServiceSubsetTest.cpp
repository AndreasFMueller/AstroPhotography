/*
 * ServiceSubsetTest.cpp -- test the ServiceSubset class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <AstroDiscovery.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

namespace astro {
namespace test {

class ServiceSubsetTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testConstructor();
	void	testTxt();

	CPPUNIT_TEST_SUITE(ServiceSubsetTest);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testTxt);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ServiceSubsetTest);

void	ServiceSubsetTest::setUp() {
}

void	ServiceSubsetTest::tearDown() {
}

void	ServiceSubsetTest::testConstructor() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() begin");
	discover::ServiceSubset	s1;
	CPPUNIT_ASSERT((!s1.has(discover::ServiceSubset::INSTRUMENTS)));
	CPPUNIT_ASSERT((!s1.has(discover::ServiceSubset::TASKS)));
	CPPUNIT_ASSERT((!s1.has(discover::ServiceSubset::GUIDING)));
	CPPUNIT_ASSERT((!s1.has(discover::ServiceSubset::IMAGES)));
	s1.set(discover::ServiceSubset::TASKS);
	CPPUNIT_ASSERT((!s1.has(discover::ServiceSubset::INSTRUMENTS)));
	CPPUNIT_ASSERT(( s1.has(discover::ServiceSubset::TASKS)));
	CPPUNIT_ASSERT((!s1.has(discover::ServiceSubset::GUIDING)));
	CPPUNIT_ASSERT((!s1.has(discover::ServiceSubset::IMAGES)));
	s1.set(discover::ServiceSubset::GUIDING);
	CPPUNIT_ASSERT((!s1.has(discover::ServiceSubset::INSTRUMENTS)));
	CPPUNIT_ASSERT(( s1.has(discover::ServiceSubset::TASKS)));
	CPPUNIT_ASSERT(( s1.has(discover::ServiceSubset::GUIDING)));
	CPPUNIT_ASSERT((!s1.has(discover::ServiceSubset::IMAGES)));
	s1.unset(discover::ServiceSubset::TASKS);
	CPPUNIT_ASSERT((!s1.has(discover::ServiceSubset::INSTRUMENTS)));
	CPPUNIT_ASSERT((!s1.has(discover::ServiceSubset::TASKS)));
	CPPUNIT_ASSERT(( s1.has(discover::ServiceSubset::GUIDING)));
	CPPUNIT_ASSERT((!s1.has(discover::ServiceSubset::IMAGES)));
	s1.unset(discover::ServiceSubset::GUIDING);
	CPPUNIT_ASSERT((!s1.has(discover::ServiceSubset::INSTRUMENTS)));
	CPPUNIT_ASSERT((!s1.has(discover::ServiceSubset::TASKS)));
	CPPUNIT_ASSERT((!s1.has(discover::ServiceSubset::GUIDING)));
	CPPUNIT_ASSERT((!s1.has(discover::ServiceSubset::IMAGES)));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() end");
}

void	ServiceSubsetTest::testTxt() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testTxt() begin");
	discover::ServiceSubset	s;
	s.set(discover::ServiceSubset::INSTRUMENTS);
	CPPUNIT_ASSERT((s.toString() == "['instruments']"));
	char	t[12];
	t[0] = 11;
	memcpy(t + 1, "instruments", 11);
	CPPUNIT_ASSERT(s.txtrecord() == std::string(t, 12));
	char	u[20];
	u[0] = 5;
	memcpy(u + 1, "tasks", 5);
	u[6] = 11;
	memcpy(u + 7, "instruments", 11);
	std::list<std::string>	l
		= discover::ServiceSubset::txtparse(std::string(u,18));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "number of records: %d", l.size());
	discover::ServiceSubset	s2(l);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "2 types: %s", s2.toString().c_str());
	CPPUNIT_ASSERT( s2.has(discover::ServiceSubset::INSTRUMENTS));
	CPPUNIT_ASSERT( s2.has(discover::ServiceSubset::TASKS));
	CPPUNIT_ASSERT(!s2.has(discover::ServiceSubset::GUIDING));
	CPPUNIT_ASSERT(!s2.has(discover::ServiceSubset::IMAGES));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testTxt() end");
}

} // namespace test
} // namespace astro
