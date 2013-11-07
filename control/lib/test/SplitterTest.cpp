
/*
 * SplitterTest.cpp -- test the device name class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

namespace astro {
namespace test {

class SplitterTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testVector();
	void	testSet();

	CPPUNIT_TEST_SUITE(SplitterTest);
	CPPUNIT_TEST(testVector);
	CPPUNIT_TEST(testSet);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SplitterTest);

void	SplitterTest::setUp() {
}

void	SplitterTest::tearDown() {
}

void	SplitterTest::testVector() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testVector() begin");
	std::vector<std::string>	data;
	split<std::vector<std::string>>("A/B/C", "/", data);
	CPPUNIT_ASSERT(data.size() == 3);
	CPPUNIT_ASSERT(data[0] == std::string("A"));
	CPPUNIT_ASSERT(data[1] == std::string("B"));
	CPPUNIT_ASSERT(data[2] == std::string("C"));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testVector() end");
}

void	SplitterTest::testSet() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSet() begin");
	std::set<std::string>	data;
	split<std::set<std::string>>("0/1/2", "/", data);
	CPPUNIT_ASSERT(data.size() == 3);
	CPPUNIT_ASSERT(data.end() != data.find(std::string("0")));
	CPPUNIT_ASSERT(data.end() != data.find(std::string("1")));
	CPPUNIT_ASSERT(data.end() != data.find(std::string("2")));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSet() end");
}

} // namespace test
} // namespace astro
